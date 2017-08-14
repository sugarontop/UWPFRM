#include "pch.h"
#include "Entry.h"
#include "content/D2DUniversalControl.h"
#include "content/D2DDriftDialog.h"
#include "content/D2DWindowMessage.h"
#include "sybil.h"
#include "SampleTest.h"

using namespace Windows::System::Threading;
using namespace V4;

void OnEntrySample1(D2DWindow* parent,FSizeF iniSz, D2CoreTextBridge* imebridge);

void OnEntry(D2DWindow* parent,FSizeF iniSz, D2CoreTextBridge* imebridge)
{
	OnEntrySample1(parent,iniSz,imebridge);
}


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




	rc.Offset( 100,100 );
	D2DChildFrame* f2 = new D2DChildFrame();
	f2->Create( parent, lstop, rc, VISIBLE, D2DChildFrame::WINSTYLE::DEFAULT,  L"f2" );
	f2->SetCanvasSize(1000,2000);
}

