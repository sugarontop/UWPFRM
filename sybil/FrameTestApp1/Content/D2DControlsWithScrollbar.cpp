#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DDriftDialog.h"

using namespace V4;

#define BARWIDTH 18

#define SCBAR(x) ((D2DScrollbar*)x.get())



void D2DControlsWithScrollbar::Create( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, bool bAutoSize, int id )
{
	FRectFBoxModel rca1(rc);

	rca1.BoderWidth_ = 1;
	
	InnerCreateWindow(pacontrol,rca1,stat,name, id);

	if (bAutoSize)
		stat_ |= AUTOSIZE;
	
	FRectFBoxModel xrc = rca1.GetContentRectZero();
	xrc.left = xrc.right - BARWIDTH;


	D2DScrollbar* Vscbar = new D2DScrollbar();
	Vscbar->Create(this,xrc,VISIBLE,NONAME );

	Vscbar_ = controls_[0];
	controls_.clear();


	xrc = rca1.GetContentRectZero();
	
	xrc.top = xrc.bottom - BARWIDTH;

	auto Hscbar = new D2DScrollbar();
	Hscbar->Create(this,xrc,VISIBLE,NONAME );
	Hscbar_ = controls_[0];
	controls_.clear();

	scrollbar_off_.height = 0;
	scrollbar_off_.width = 0;
	scale_ = 1.0f;

}




int D2DControlsWithScrollbar::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( IsHide())
		return 0;
	int ret = 0;
		
	switch( message )
	{
		case WM_PAINT:
		{			
			D2DContext& cxt = *(d->cxt());

			D2DMatrix mat(cxt);	
		
			mat.PushTransform();
			//mat.Scale(scale_,scale_);
			
			FRectF rcborder = rc_.GetBorderRect();
			

			auto br = ( stat_ & BORDERLESS ? cxt.transparent : cxt.black );

			cxt.cxt->DrawRectangle( rcborder, br );
			cxt.cxt->FillRectangle( rcborder, cxt.white );
			
			FRectF rc = rc_.GetContentRect();
			D2DRectFilter f(cxt, rc );
			
			mat.Offset( rcborder.left, rcborder.top );		
			mat_ = mat; // Ž©À•W(¶ã0,0)‚Ìmatrix
			
			mat.PushTransform();
			{
				mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );

				DefPaintWndProc(d,message,wp,lp); 
			}
			mat.PopTransform();

			Vscbar_->WndProc(d,WM_PAINT,wp,lp);
			Hscbar_->WndProc(d,WM_PAINT,wp,lp);

			mat.PopTransform();
			return 0;
		}
		break;

		case WM_MOUSEMOVE:
		{
			FPointF pt = mat_.DPtoLP( lp);
			pt.x += rc_.left;
			pt.y += rc_.top;

			if ( rc_.PtInRect( pt ))				
				ret = DefWndScrollbarProc(d,message,wp,lp);
	

		}
		break;				
		case WM_MOUSEWHEEL:
		{
			FPointF pt = mat_.DPtoLP(lp);
			pt.x += rc_.left;
			pt.y += rc_.top;

			if ( rc_.PtInRect( pt ))		
			{
				ret = DefWndScrollbarProc(d,message,wp,lp);
			}
		}
		break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_CAPTURECHANGED:		
		{
			FPointF pt = mat_.DPtoLP( FPointF(lp));
			pt.x += rc_.left;
			pt.y += rc_.top;
			
			if ( rc_.PtInRect( pt ))
			{				
				ret = DefWndScrollbarProc(d,message,wp,lp);
			}
		}
		break;
		
		//case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		{						
			if ( stat_ & AUTOSIZE )
			{
				FRectF rc = parent_control_->GetRect().GetContentRect();
				rc_ = rc.ZeroRect();
			}				


			ret = DefWndScrollbarProc(d,message,wp,lp);
		}
		break;

		default :
			ret = DefWndProc(d,message,wp,lp);
	}
		

	return ret;
}

int D2DControlsWithScrollbar::DefWndScrollbarProc(D2DWindow* d, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam)
{
	int ret = D2DControls::DefWndProc(d,message,wParam,lParam);
	
	if ( ret == 0 )
		ret = Vscbar_->WndProc(d,message,wParam,lParam);
	if ( ret == 0 )
		ret = Hscbar_->WndProc(d,message,wParam,lParam);

	return ret;
}
void D2DControlsWithScrollbar::ShowScrollbar( SCROLLBAR_TYP typ, bool visible )
{
	if ( visible )
	{
		if ( typ == SCROLLBAR_TYP::VSCROLLBAR )		
			SCBAR(Vscbar_)->Visible();
		else if ( typ == SCROLLBAR_TYP::HSCROLLBAR )		
			SCBAR(Hscbar_)->Visible();
	}
	else
	{
		if ( typ == SCROLLBAR_TYP::VSCROLLBAR )		
			SCBAR(Vscbar_)->Hide();
		else if ( typ == SCROLLBAR_TYP::HSCROLLBAR )		
			SCBAR(Hscbar_)->Hide();
	}
}

void D2DControlsWithScrollbar::SetTotalSize( float cx, float cy )
{
	if (cy > 0 )
	{
		cy = max(rc_.Height(), cy );	
		SCBAR(Vscbar_)->SetTotalSize( cy );
	}


	if ( cx > 0 )
	{
		cx = max(rc_.Width(), cx );	
		SCBAR(Hscbar_)->SetTotalSize( cx );
	}


	//

	auto& vinfo = SCBAR(Vscbar_)->Info();
	auto& hinfo = SCBAR(Hscbar_)->Info();

	bool bl1 = true, bl2 = true;

	if ( vinfo.total_height <= rc_.Height())
	{
		SCBAR(Vscbar_)->Hide();
		bl1 = false;
	}
	if ( hinfo.total_height <= rc_.Width())
	{
		SCBAR(Hscbar_)->Hide();
		bl2 = false;
	}

	SCBAR(Vscbar_)->OtherHand(bl2);
	SCBAR(Hscbar_)->OtherHand(bl1);

}


void D2DControlsWithScrollbar::UpdateScrollbar(D2DScrollbar* bar)
{
	auto& info = bar->Info();
	
	if ( info.bVertical )
	{
		scrollbar_off_.height = info.position / info.thumb_step_c;
	}
	else
	{
		scrollbar_off_.width = info.position / info.thumb_step_c;
	}

}


void D2DControlsWithScrollbar::OnDXDeviceLost() 
{ 
	SCBAR(Vscbar_)->OnDXDeviceLost();
	SCBAR(Hscbar_)->OnDXDeviceLost();
}
void D2DControlsWithScrollbar::OnDXDeviceRestored()  
{ 
	SCBAR(Vscbar_)->OnDXDeviceRestored();
	SCBAR(Hscbar_)->OnDXDeviceRestored();
}





