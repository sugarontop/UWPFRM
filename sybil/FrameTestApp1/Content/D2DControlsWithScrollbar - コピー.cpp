#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DDriftDialog.h"

using namespace V4;
#define BARWIDTH 13

void D2DControlsWithScrollbar::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, WINSTYLE ws, LPCWSTR name, int controlid )
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, controlid);
	ws_ = ws;

	D2DScrollbar* vscroll = (ws & WINSTYLE::VSCROLL ? new D2DScrollbar() : nullptr );
	D2DScrollbar* hscroll = (ws & WINSTYLE::HSCROLL ? new D2DScrollbar() : nullptr );

	FRectF vrc, hrc;
	offx_ = 0;
	offy_ = 0;
	szCanvas_ = rc.GetBorderRect().Size();

	float sw = BARWIDTH;
	float w = rc.Width(), h = rc.Height();
	vrc.SetRect( w-sw, 0,w, h);
	hrc.SetRect( 0, h-w, w, h );

	if ( vscroll )	
	{
		vscroll->Create( parent,this, vrc, VISIBLE, D2DScrollbar::WINSTYLE::VSCROLL, L"noname" );			
		vscroll_ = controls_[0];
		controls_.clear(); //  scrollbar‚Í•ÊŠÇ—‚Æ‚·‚é
	}
	
	if ( hscroll )	
	{
		hscroll->Create( parent,this, hrc, VISIBLE, D2DScrollbar::WINSTYLE::HSCROLL, L"noname" );
		hscroll_ = controls_[0];
		controls_.clear(); //  scrollbar‚Í•ÊŠÇ—‚Æ‚·‚é
	}	
}
int D2DControlsWithScrollbar::WndProc(D2DWindow* d, int message, int wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( !(stat_ & VISIBLE) )
		return 0;
		
	int ret = 0;

	switch( message )
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());

			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();
			auto rc = rc_.GetBorderRect();

			cxt.cxt->DrawRectangle( rc, cxt.black );
			cxt.cxt->FillRectangle( rc, cxt.white );

			
			mat.Offset(rc_.left, rc_.top);

			rc = rc_.GetBorderRect().ZeroRect();
			if ( vscroll_ )
			{
				rc.right -= BARWIDTH;
				vscroll_->WndProc(d,message,wp,lp);
			}
			if ( hscroll_ )
			{
				rc.bottom -= BARWIDTH;
				hscroll_->WndProc(d,message,wp,lp);
			}


			D2DRectFilter fil(cxt, rc );
			mat.Offset(offx_, offy_);
			DefPaintWndProc(d,message,wp,lp);

			mat.PopTransform();
		}
		break;
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEWHEEL:
			if ( vscroll_ ) ret = vscroll_->WndProc(d,message,wp,lp);
			

		break;

		default :
			ret = DefWndProc(d,message,wp,lp);

	}


	return ret;


}
void D2DControlsWithScrollbar::SetCanvas(float w, float h)
{
	if ( w > 0 ) szCanvas_.width = w;
	if ( h > 0 ) szCanvas_.height = h;

	if ( vscroll_ )
	{
		vscroll_->SetCanvas(h);


	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DScrollbar::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, WINSTYLE ws, LPCWSTR name, int local_id )
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, local_id);
	ws_ = ws;



	scinfo_.h = rc.Height();
	scinfo_.canvash = rc.Height();
	scinfo_.topbtnh = 20;
	scinfo_.bottombtnh = 20;
	scinfo_.offset_pos = 0;
	Calc();


}

void D2DScrollbar::Calc()
{
	if ( scinfo_.h >= scinfo_.canvash )
	{
		scinfo_.calc_step = 1;
		scinfo_.calc_thumbh = scinfo_.h-scinfo_.topbtnh-scinfo_.bottombtnh;
		scinfo_.offset_pos = 0;
	}
	else
	{
		float ah = scinfo_.h-scinfo_.topbtnh-scinfo_.bottombtnh;
		float off = scinfo_.canvash - ah;

		float min_thumbh = 5;
		float ah2 = ah - min_thumbh;

		scinfo_.calc_step = max(1.0, off / ah2);

		if ( scinfo_.calc_step == 1.0 )
			scinfo_.calc_thumbh = off - ah;



	}

}

int D2DScrollbar::WndProc(D2DWindow* d, int message, int wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	int ret = 0;

	switch( message )
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());

			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();

			mat.Offset(rc_.left, rc_.top);

			DrawBar( cxt, ws_ == WINSTYLE::VSCROLL );

			mat.PopTransform();
		}
		break;
		case WM_MOUSEWHEEL:
		{
			Windows::UI::Core::PointerEventArgs^ arg = (Windows::UI::Core::PointerEventArgs^)lp;
			D2DControlsWithScrollbar* sc = dynamic_cast<D2DControlsWithScrollbar*>(parent_control_);
			if ( sc )
			{
				float delta = arg->CurrentPoint->Properties->MouseWheelDelta / 120 * 2;

				if ( ws_ == VSCROLL )
					sc->offy_ += delta;
			
				ret = 1;
				d->redraw();
			}
					


		}
		break;
	}


	return ret;
}
void D2DScrollbar::SetCanvas( float h )
{
	


}
void D2DScrollbar::DrawBar( D2DContext& cxt, bool vscroll )
{
	cxt.cxt->FillRectangle( rc_.ZeroRect(), cxt.gray );
}