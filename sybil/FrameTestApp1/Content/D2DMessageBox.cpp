#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
using namespace V4;


static FSizeF TextLayoutSize(IDWriteTextLayout* title )
{
	DWRITE_TEXT_METRICS tm;
	title->GetMetrics(&tm);
	return FSizeF(tm.width, tm.height);
}
static void CreateSingleLineTextLayout( D2DContext* pcxt, LPCWSTR str, int strlen, IDWriteTextLayout** ret )
{
	ComPTR<IDWriteTextLayout> layout;	
	auto tf = pcxt->cxtt.textformat;
	auto h = tf->GetFontSize();
	auto hr = pcxt->cxtt.wfactory->CreateTextLayout( str, strlen, tf, 9999, h, &layout );
	_ASSERT( hr == S_OK );
	
	layout.p->AddRef();
	*ret = (layout.p);	
}


// static function ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DMessageBox::Show(D2DWindow* parent, const FRectF& rc,  LPCWSTR title, LPCWSTR msg, int typ )
{
	_ASSERT(parent);
	auto pc = dynamic_cast<D2DControls*>(parent);
	_ASSERT(pc);

	D2DMessageBox* p = new D2DMessageBox();
	
	FRectF rc1( rc.LeftTop(), FSizeF(326,200));
	
	p->typ_ = typ;
	p->Create( pc, rc1, VISIBLE, NONAME,-1 );
	
	p->title_.Release();
	p->msg_.Release();
	CreateSingleLineTextLayout(parent->cxt(), title, wcslen(title), &(p->title_) );
	CreateSingleLineTextLayout(parent->cxt(), msg, wcslen(msg), &(p->msg_) );	
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DMessageBox::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid )
{
	InnerCreateWindow(pacontrol,rc,stat,name, controlid);

	FRectF rcOk(10,150,FSizeF(100,26));

	auto parent = pacontrol->GetParentWindow();

	if ( typ_ == MB_OKCANCEL || typ_ == MB_OK )
	{		
		D2DButton* okb = new D2DButton();
		okb->Create( parent, this, rcOk, VISIBLE, L"OK", L"noname" );
		okb->OnClick_ = [this](D2DButton* b)
		{
			result_ = IDOK;
			stat_ &= ~VISIBLE;
			parent_control_->ReleaseCapture(); 
			DestroyControl();
		};
	}

	if ( typ_ == MB_OKCANCEL )
	{
		FRectF rcCancel(rcOk);
		rcCancel.Offset( 200, 0 );
		D2DButton* canb = new D2DButton();
		canb->Create( parent, this, rcCancel, VISIBLE, L"CANCEL", L"noname" );
		canb->OnClick_ = [this](D2DButton* b)
		{
			result_ = IDCANCEL;
			stat_ &= ~VISIBLE;
			parent_control_->ReleaseCapture(); 
			DestroyControl();
		};
	}


	pacontrol->SetCapture( this );
	result_ = IDCANCEL;

	CreateSingleLineTextLayout(parent->cxt(), L"title", 5, &(title_) );
	CreateSingleLineTextLayout(parent->cxt(), L"msg", 3, &(msg_) );	

}
int D2DMessageBox::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( this->IsHide())
		return 0;

	int ret = 0;

	switch ( message )
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());

			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();
			mat.Offset(rc_.left, rc_.top);


			FRectF rc = rc_.ZeroRect();
				
			cxt.cxt->DrawRectangle( rc, cxt.black );
			cxt.cxt->FillRectangle( rc, cxt.ltgray );

			// draw title
			{
				FRectF rca(0,0, TextLayoutSize(title_));
				rca.CenterHRect( rc );
				cxt.cxt->FillRectangle( FRectF( 0,0,rc.right, rca.bottom ), cxt.bluegray );
				cxt.cxt->DrawTextLayout( rca.LeftTop(), title_, cxt.white );
			}

			// draw message
			{
				FRectF rca(0,0, TextLayoutSize(msg_));
				rca.CenterRect( rc );			
				cxt.cxt->DrawTextLayout( rca.LeftTop(), msg_, cxt.black );	
			}


			D2DControls::DefPaintWndProc( d, message, wp, lp );


			mat.PopTransform();

		
			return 0;
		}
		break;		
		case WM_KEYDOWN :
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;

			switch( arg->VirtualKey )
			{
				case Windows::System::VirtualKey::Escape:
				{
					parent_control_->ReleaseCapture();
					DestroyControl();
					ret = 1; 

					return ret;
				}
				break;
				//case Windows::System::VirtualKey::T:
				//{
				//	FRectF rc( 0,0, FSizeF(300,300));

				//	D2DMessageBox::Show( parent_, rc, L"test", L"this is test",0 );
				//	ret = 1;
				//}
				//break;
			}

			ret = DefWndProc( d, message, wp, lp );
		}
		break;
		case WM_SETTEXT:
		{
			WParameterString* ws = (WParameterString*)wp;

			title_.Release();
			msg_.Release();
			
			CreateSingleLineTextLayout(d->cxt(), ws->str2, wcslen(ws->str2), &(title_) );
			CreateSingleLineTextLayout(d->cxt(), ws->str1, wcslen(ws->str1), &(msg_) );	

			::SysFreeString(ws->str2);
			::SysFreeString(ws->str1);
			d->redraw();
			ret = 1;
		}
		break;


		default :
			ret = DefWndProc( d, message, wp, lp );
	}

	if ( ret == 0 )
		ret = (parent_control_->GetCapture() == this ? 1 : 0);

	return ret;
}
