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
#include "Content/D2DDevelop.h"


using namespace Windows::System::Threading;
using namespace V4;




void OnEntrySample1(D2DWindow* parent,FSizeF iniSz, D2CoreTextBridge* imebridge);

bool JsOnEntryJavascript(js_context& ret);
void JsOnAppEixt();

D2DWindow* gparent;
D2CoreTextBridge*  gimebridge;
D2DControls* gf1;

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
	bool bl = sybil::WriteFileWStore( L"error.log", (byte*)src, len, 1 );
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


	const float left_control_width = 100;

	{
		D2D1_COLOR_F clr[3];
		clr[0] = D2RGB(85,41,41);
		clr[1] = D2RGB(126,71,71);
		clr[2] = D2RGB(225,201,201);


		FRectF rc1(0,0,left_control_width,100);
		D2DVerticalbarControls* fleft = new D2DVerticalbarControls();
		fleft->Create( ls, rc1, VISIBLE, L"dumy1", clr,3 );


		fleft->wmsize_ = [left_control_width](D2DControl* cs)->FRectF {

			auto rc = cs->GetParentControl()->GetRect();

			rc.left = 0;
			rc.right = left_control_width;

			return rc;
		};
	}


	D2DTabControls* tab = new D2DTabControls();
	tab->Create(ls, FRectF(left_control_width,0,FSizeF(100,100)), STAT::VISIBLE, NONAME );

	tab->wmsize_ = [left_control_width](D2DControl* cs)->FRectF {

			auto rc = cs->GetParentControl()->GetRect();

			rc.left = left_control_width;

			return rc;
	};

	D2DControls* lstop = tab;


	D2DTransparentControls* mount = new D2DTransparentControls();
	mount->Create(lstop, L"m1",1);
  	 
	
		
	FRectF rc(50,50,FSizeF(900,800));
	D2DChildFrame2* f1 = new D2DChildFrame2();
	f1->Create( mount, rc, VISIBLE, L"top_child_controls" );
	f1->SetCanvasSize(1000,2000);
	 

	rc.Offset(100,100);
	D2DChildFrame2* f1a = new D2DChildFrame2();
	f1a->Create( mount, rc, VISIBLE, L"no2" );
	f1a->SetCanvasSize(1000,2000);



	gf1 = f1; // test


	// AppApi.cpp
	gparent = main;
	gimebridge = imebridge;

	auto& caret = Caret::GetCaret();

	rc.SetRect(500,500,FSizeF(500,500));

	   	
	DllBridge drd;
	drd.ctrls = f1;
	drd.caret = &caret;
	drd.factory = D2DControlfactory;
	drd.script = &script;

	XApp1::Class1::pin_hole( (INT_PTR)&drd);




	rc.SetRect(0,0,500,600);
	D2DChildFrame2* f2 = new D2DChildFrame2();
	f2->Create( lstop, rc, VISIBLE, L"second",2 );
	f2->SetCanvasSize(1000,2000);
	
	
}



 


 
