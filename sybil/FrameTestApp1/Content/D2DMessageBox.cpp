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
	p->Create( parent, pc, rc1, VISIBLE, NONAME,-1 );
	
	
	CreateSingleLineTextLayout(parent->cxt(), title, wcslen(title), &(p->title_) );
	CreateSingleLineTextLayout(parent->cxt(), msg, wcslen(msg), &(p->msg_) );	
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DMessageBox::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid )
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, controlid);

	FRectF rcOk(10,150,FSizeF(100,26));

	if ( typ_ == MB_OKCANCEL || typ_ == MB_OK )
	{		
		D2DButton* okb = new D2DButton();
		okb->Create( parent, this, rcOk, VISIBLE, L"OK", L"noname" );
		okb->OnClick_ = [this](D2DButton* b)
		{
			result_ = IDOK;
			stat_ &= ~VISIBLE;
			parent_control_->ReleaseCapture(-1);
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
			parent_control_->ReleaseCapture(-1);
			DestroyControl();
		};
	}


	pacontrol->SetCapture( this );
	result_ = IDCANCEL;

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
					parent_control_->ReleaseCapture(-1);

					
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

		default :
			ret = DefWndProc( d, message, wp, lp );
	}

	if ( ret == 0 )
		ret = (parent_control_->GetCapture() == this ? 1 : 0);

	return ret;
}
