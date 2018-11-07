#include "pch.h"
#include "Entry.h"
#include "content/D2DUniversalControl.h"
#include "content/D2DTextbox.h"
#include "content/D2DDriftDialog.h"
#include "content/D2DWindowMessage.h"
#include "sybil.h"
#include "Content/script.h"
#include "Content/CJsValueRef.h"
#include "Content/InvokeHelper.h"
#include "Content/script.h"
#include "Content/CJsValueRef.h"
#include "Content/InvokeHelper.h"



using namespace Windows::System::Threading;
using namespace V4;




void OnEntrySample1(D2DWindow* parent,FSizeF iniSz, D2CoreTextBridge* imebridge);

bool JsOnEntryJavascript(js_context& ret);
void JsOnAppEixt();

D2DWindow* gparent;
D2DChildFrame* gf1;
D2CoreTextBridge*  gimebridge;

std::map<IDispatch*,D2DControl*> gWindowMap;

static js_context gapp_script_context;


class Javascript : public Script
{
	public :
		
		Javascript(){};
		virtual bool ExecBSTR( LPCWSTR function_name, BSTR* bsret )
		{
			JsValueRef ret;
			if ( 0 != js_run(gapp_script_context, function_name,  &ret))
				return false;
			CJsValueRef c(ret);

			*bsret =c.ToBSTR();
			return true;

		}

};


static Javascript script;


 void WINAPI _com_issue_error(long er)
 {
	// not implement.
 }
 LPCSTR ToUtf8( LPCWSTR str, int* pcblen )
{
	int& cblen = *pcblen;
	cblen = ::WideCharToMultiByte( CP_UTF8, 0, str,wcslen(str), 0,0,0,0);
	
	byte* cb = new byte[cblen+1];

	::WideCharToMultiByte( CP_UTF8, 0, str,wcslen(str), (LPSTR)cb,cblen,0,0);

	cb[cblen]=0;

	return (LPCSTR)cb;
}

bool WriteLogFile( LPCWSTR content )
{	
	int len;
	auto src = ToUtf8( content, &len);		
	bool bl = sybil::WriteFile( L"error.log", (byte*)src, len, 1 );
	delete src;
	return bl;
}


void OnEntry(D2DWindow* parent,FSizeF iniSz, D2CoreTextBridge* imebridge)
{	
	bool bl = WriteLogFile(L"gofire");

	try
	{
		bool bl = JsOnEntryJavascript(gapp_script_context);
		OnEntrySample1(parent,iniSz,imebridge);
	}	
	catch( std::wstring errmsg)
	{
		WriteLogFile( errmsg.c_str());
	}
}

void OnExit()
{
	JsOnAppEixt();

} 
D2DControl* WINAPI D2DControlfactory(LPCWSTR typ, D2DControls* parent, Caret* ca, FRectF rc, LPCWSTR nm)
{
	if ( wcscmp( typ, L"textbox") == 0 )
	{
		D2CoreTextBridge*  ime =  dynamic_cast<D2DMainWindow*>(parent->GetParentWindow())->GetImeBridge();
		D2DTextbox* tx = new D2DTextbox(*ime, *ca);
		tx->Create( parent->GetParentWindow(), parent, rc, VISIBLE, nm );	
		return tx;
	}
	return nullptr;
} 

void f1draw( D2DContext&cxt, D2D1_RECT_F& rc )
{	
	ComPTR<ID2D1SolidColorBrush> br;
	cxt.cxt->CreateSolidColorBrush( D2RGB(42,53,66), &br);

	cxt.cxt->FillRectangle(rc, br);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void OnEntrySample1(D2DWindow* parent,FSizeF iniSz, D2CoreTextBridge* imebridge)
{
	D2DMainWindow* main = dynamic_cast<D2DMainWindow*>(parent);
	main->imebridge_ = imebridge;


	D2DControls* ls = dynamic_cast<D2DControls*>(parent);
	D2DControls* lstop = ls;
		
	FRectF rc(50,50,FSizeF(900,800));
	D2DChildFrame* f1 = new D2DChildFrame();
	f1->Create( parent, lstop, rc, VISIBLE, D2DChildFrame::WINSTYLE::DEFAULT,  L"top_child_controls" );
	f1->SetCanvasSize(1000,2000);


	// AppApi.cpp
	gparent = main;
	gimebridge = imebridge;
	gf1 = f1;

	auto& caret = Caret::GetCaret();

	rc.SetRect(500,500,FSizeF(500,500));

	//{
	//	FRectF rcx(300,50,FSizeF(600,26));
	//	D2DTextbox* tx = new D2DTextbox(*imebridge, caret);
	//	tx->Create(parent, f1, rcx, VISIBLE, L"noname" );
	//	tx->SetText( L"‚·‚×‚ÄDirect2D‚É‚æ‚é•`‰æ");
	//	
	//}


	//FRectFBoxModel rcm = rc;
	//rcm.Padding_.Set(5);
	//D2DChildControls* gp = new D2DChildControls();
	//gp->Create( parent, f1, rcm, VISIBLE, L"f1c");
	//gp->SetBackground( f1draw );

	   	
	DllBridge drd;
	drd.ctrls = f1;
	drd.caret = &caret;
	drd.factory = D2DControlfactory;
	drd.script = &script;

	XApp1::Class1::pin_hole( (INT_PTR)&drd);

}



 


 
