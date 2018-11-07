#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DDriftDialog.h"
#include "D2DDevelop.h"
#include "sybil.h"

using namespace V4;

FPointF ptold;

void D2DChildControls::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id )
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, local_id);
	auto wfac = parent_->cxt()->cxtt.wfactory;
	auto tf = parent_->cxt()->cxtt.textformat;

	//sybil::CreateSingleTextLayout( wfac, name_.c_str(), name_.length(), tf, &title_ );
}



int D2DChildControls::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if (IsHide() ) 
		return 0;

	int ret = 0;
	
	
	switch( message )
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());

			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();
			FRectF rcb = rc_.GetBorderRect();
			
			cxt.cxt->DrawRectangle(rcb, cxt.black );

			//auto br = Brush(cxt, D2RGB(170,170,250));
			rcb = rc_.GetContentRect();
			//cxt.cxt->FillRectangle(rcb, br);

			back_ground_(cxt, rcb);


			mat.PushTransform();
			mat.Offset(rcb.left, rcb.top);			
			DefPaintWndProc(d,WM_PAINT,wp,nullptr); 
			mat.PopTransform();
			
			
			return 0;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(lp);
			mode_ = MODE::NONE;
			if ( rc_.PtInRect(pt ))
			{
				ret = DefWndProc(d,message,wp,lp); 

				if ( ret == 0 )
				{
					mode_ = MODE::MOVE;
					GetParentControl()->SetCapture(this);
					ret = 1;
					ptold = pt;
				}
			}
			else
			{
				if ( this->GetCapture() == this )
				{
					GetParentControl()->ReleaseCapture(this);
					ret = 0;
				}
			}

		}
		break;
		case WM_MOUSEMOVE :
		{
			if ( GetParentControl()->GetCapture() == this && mode_ == MODE::MOVE )
			{
				FPointF pt = mat_.DPtoLP(lp);
				//if ( rc_.PtInRect(pt))
				{
					D2DMainWindow::SetCursor(CURSOR_ARROW);

				

					rc_.Offset( pt.x - ptold.x, pt.y - ptold.y );
					ptold = pt;

					ret = 1;
					d->redraw();
				}
			}

			if ( ret == 0 )
				ret = DefWndProc(d,message,wp,lp); 

		}
		break;
		case WM_LBUTTONUP:
		{
			if ( GetParentControl()->GetCapture() == this && mode_ == MODE::MOVE )
			{
				ReleaseCapture(this);
				ret = 0;
				mode_ = MODE::NONE;
			}
			else
				ret = DefWndProc(d,message,wp,lp); 

		}
		break;		
	}
	
	if ( WM_D2D_USERCD <= message )
	{
		
		if ( Extention_.find(message) != Extention_.end())
			ret = Extention_[message](message,wp,lp);

	}
	   

	if ( ret == 0 )
		ret = DefWndProc(d,message,wp,lp); 


	return ret;	
}