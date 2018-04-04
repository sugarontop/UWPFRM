#include "pch.h"
#include "Entry.h"
#include "content/D2DUniversalControl.h"
#include "content/D2DDriftDialog.h"
#include "content/D2DWindowMessage.h"
#include "sybil.h"
#include "SampleTest.h"
#include "Content/script.h"
#include "Content/CJsValueRef.h"
#include "Content/InvokeHelper.h"

using namespace Windows::System::Threading;
using namespace V4;


js_context app_script_context;

void OnEntrySample1(D2DWindow* parent,FSizeF iniSz, D2CoreTextBridge* imebridge);
void OnEntryJavascript();

D2DWindow* gparent;
D2DChildFrame* gf1;
D2CoreTextBridge*  gimebridge;
std::map<IDispatch*,D2DControl*> gWindowMap;



 void WINAPI _com_issue_error(long er)
 {
	// not implement.
 }


void OnEntry(D2DWindow* parent,FSizeF iniSz, D2CoreTextBridge* imebridge)
{	
	
	OnEntrySample1(parent,iniSz,imebridge);


}

void OnExit()
{
	js_app_exit( app_script_context );

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
	f1->Create( parent, lstop, rc, VISIBLE, D2DChildFrame::WINSTYLE::DEFAULT,  L"f1" );
	f1->SetCanvasSize(1000,2000);

	{
		FRectF rcx(100,50,FSizeF(600,26));
		D2DTextbox* tx = new D2DTextbox(*imebridge);
		tx->Create(parent, f1, rcx, VISIBLE, L"noname" );
		tx->SetText( L"‚·‚×‚ÄDirect2D‚É‚æ‚é•`‰æ");

		rcx.SetRect(100,150,FSizeF(300,300));
		CreateRectBox( f1, rcx );
	}
	 
	


	D2DButton* msgbox_btn = new D2DButton();
	msgbox_btn->Create( parent, f1, FRectF(100,100,FSizeF(200,26)), VISIBLE, L"sample Messagebox", L"noname" );
	msgbox_btn->OnClick_ = [](D2DButton* b)
	{
		FRectF rc( 100,200, FSizeF(200,200));
		D2DMessageBox::Show( b->GetParentWindow(), rc, L"[title area]", L"Hello world.", MB_OKCANCEL );
	};


	D2DButton* get_btn = new D2DButton();
	get_btn->Create( parent, f1, FRectF(400,100,FSizeF(200,26)), VISIBLE, L"internet GET", L"noname" );
	get_btn->OnClick_ = [main](D2DButton* b)
	{

		BSTRPtr url = L"https://github.com/sugarontop/UWPFRM/raw/master/sybil/FrameTestApp1/sample-msft-20170814.csv";

		sybil::ResponseData* data = new sybil::ResponseData();
		sybil::ResponseDataInit(data);
		
		D2DMainWindow::timerfunc complete_function = [data,main](int, bool* IsComplete){

			// Here is gui thread.

			if ( data->result > 0 )
			{
				if ( data->result == 200 )
				{
					BSTR bs = data->data;

					XST x;
					x.tag = 0;
					x.data = bs;

					main->SendMessage(WM_D2D_INTERNET_GET_COMPLETE, (INT_PTR)&x, nullptr );
					
				}

				*IsComplete = true;
				ResponseDataClear(data);
				delete data;
			}
			else
			{
				// wait for server response.
			}
		};

		main->timerfuncs_.push_back( complete_function );


		sybil::GETInternet( url, nullptr, 0, data );

	};

	rc.Offset( 100,100 );
	D2DChildFrame* f2 = new D2DChildFrame();
	f2->Create( parent, lstop, rc, VISIBLE, D2DChildFrame::WINSTYLE::DEFAULT,  L"f2" );
	f2->SetCanvasSize(1000,2000);


	
	
	gparent = parent;
	gf1 = f1;
	gimebridge=imebridge;
	
	OnEntryJavascript();



}



 


 
