#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DDriftDialog.h"
#include "D2DDevelop.h"

using namespace V4;

#include "sybil.h"

//static D2DColor ColorPallet_1[] = { D2DColor(0x59202c),D2DColor(0x7f2c3e),D2DColor(0xa6385e),D2DColor(0x898989),D2DColor(0xa2a2a2) };
static D2DColor ColorPallet_1[] = { D2DColor(0x28716f),D2DColor(0x349795),D2DColor(0x40bfbc),D2DColor(0x999999),D2DColor(0xab2b2b2) };

#define DEEPCOLOR	DRGB(ColorPallet_1[0])
#define CENTERCOLOR	DRGB(ColorPallet_1[1])
#define LIGHTCOLOR	DRGB(ColorPallet_1[2])
#define TEXTCOLOR	DRGB(ColorPallet_1[3])
#define LIGHTTEXTCOLOR	DRGB(ColorPallet_1[4])

#define BARWIDTH 18.0f

#define SCBAR(x) ((D2DScrollbar*)x.get())

#define TITLEBAR_HIGHT 30.0f

static FRectFBoxModel VScrollbarRect( const FRectFBoxModel& rc )
{
	FRectFBoxModel xrc(rc);
	xrc.left = xrc.right - BARWIDTH;
	xrc.top += TITLEBAR_HIGHT;

	return xrc;
}
static FRectFBoxModel HScrollbarRect( const FRectFBoxModel& rc )
{
	FRectFBoxModel xrc(rc);
	xrc.top = xrc.bottom - BARWIDTH;

	return xrc;
}

void D2DChildFrameEX::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, WINSTYLE ws,LPCWSTR name, int local_id )
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, local_id);
	md_ = MODE::NONE;
	wmd_ = WINDOWMODE::NORMAL;
	auto wfac = parent_->cxt()->cxtt.wfactory;
	auto tf = parent_->cxt()->cxtt.textformat;
	scale_ = 1.0f;

	sybil::CreateSingleTextLayout( wfac, name_.c_str(), name_.length(), tf, &title_ );

	/////////////////
	FRectFBoxModel xrc = VScrollbarRect(rc.GetContentRectZero());

	D2DScrollbar* Vscbar = new D2DScrollbar();
	Vscbar->Create(parent,this,xrc,VISIBLE,NONAME );

	Vscbar_ = controls_[0];
	controls_.clear();


	xrc = HScrollbarRect(rc.GetContentRectZero());
		

	auto Hscbar = new D2DScrollbar();
	Hscbar->Create(parent,this,xrc,VISIBLE,NONAME );
	Hscbar_ = controls_[0];
	controls_.clear();

	scrollbar_off_.height = 0;
	scrollbar_off_.width = 0;






}
void D2DChildFrameEX::OnCaptureUpdate()
{
	if ( IsCaptured() )
	{
		title_.Release();
		sybil::CreateSingleTextLayout( GetDWFactory(), L"Active",6, GetTextFormat(), &title_ );
	}
	else
	{
		title_.Release();		
		sybil::CreateSingleTextLayout( GetDWFactory(), name_.c_str(), name_.length(), GetTextFormat(), &title_ );
	}
}
void D2DChildFrameEX::DrawDriftRect(D2DWindow* d, D2DContext& cxt)
{
	if ( drift_ )
	{
		FRectF rcz = drift_->dstRect;
		FRectF rc_calc;
		drift_->cc = sybil::DrawDriftRect( drift_->cc, &rc_calc, cxt.tickcount_, rc_, rcz, 500 );

		if ( drift_->cc == nullptr )
		{								
			// mission completed
			
			if ( drift_->completed )
				drift_->completed();
			
			drift_.reset();
		}

		rc_ = rc_calc;

		WndProc( d, WM_SIZE, 0, nullptr );
							
		d->redraw();

	}
}
void D2DChildFrameEX::DrawTitle(D2DContext& cxt, const FRectF& rc )
{
	FRectF rc1 = rc.ZeroRect();

	cxt.cxt->DrawRectangle( rc1, cxt.black );
	cxt.cxt->FillRectangle( rc1, cxt.white );
	rc1.bottom = rc1.top + TITLEBAR_HIGHT;

	if ( IsCaptured() )
	{
		cxt.cxt->FillRectangle( rc1, DRGB(DEEPCOLOR)); // cxt.bluegray );
		sybil::DrawTextLayoutCenter( cxt.cxt, rc1, title_,  cxt.white  ); 
	}
	else
	{
		cxt.cxt->FillRectangle( rc1, (LIGHTCOLOR)); // cxt.gray );
		sybil::DrawTextLayoutCenter( cxt.cxt, rc1, title_,  cxt.black  ); 
	}

	// left side button
	{
		FRectF rcb(0,0,10,TITLEBAR_HIGHT);
		cxt.cxt->FillRectangle( rcb, LIGHTCOLOR ); // maxsize
		rcb.Offset(10,0);
		cxt.cxt->FillRectangle( rcb, CENTERCOLOR ); // property


		rcb.Offset(10,0);
		cxt.cxt->FillRectangle( rcb, DRGB(D2DColor(0x59202c)) ); // close





	}

}
void D2DChildFrameEX::SetScale( float scale )
{
	scale_ = scale;
	parent_->redraw();
}
int D2DChildFrameEX::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if (IsHide() ) 
		return 0;

	int ret = 0;
	
	
	switch( message )
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());

			DrawDriftRect(d, cxt);
			
			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();
			FRectF rcb = rc_.GetBorderRect();
			
			mat.Offset(rcb.left, rcb.top);			
			DrawTitle( cxt, rcb );


			auto rcb1 = rcb.ZeroRect();
			rcb1.top += TITLEBAR_HIGHT;
			D2DRectFilter f(cxt, rcb1 ); 
			
			mat.PushTransform();
			{
				mat.Scale(scale_,scale_);
				mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );

				DefPaintWndProc(d,message,wp,lp); 
			}
			mat.PopTransform();
			
			Vscbar_->WndProc(d,WM_PAINT,0,lp);
			Hscbar_->WndProc(d,WM_PAINT,0,lp);

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(lp);
			if ( rc_.PtInRect(pt ))
			{
				md_ = MODE::NONE;
				auto rc_title_bar = rc_;
				rc_title_bar.bottom = rc_title_bar.top + TITLEBAR_HIGHT;
				
				if ( rc_title_bar.PtInRect(pt))			
				{
					if ( TB_MouseWindowResize( FMODE::TRY,rc_title_bar, pt ) )
					{
						TB_MouseWindowResize( FMODE::DO,rc_title_bar, pt );
						ret = 1;
					}
					else if ( TB_DlgWindowProperty(FMODE::TRY,rc_title_bar, pt ) )
					{
						TB_DlgWindowProperty(FMODE::DO,rc_title_bar, pt );
						ret = 1;
					}
					else if ( TB_WindowClose(FMODE::TRY,rc_title_bar, pt ) )
						TB_WindowClose(FMODE::DO,rc_title_bar, pt );
					else 
					{
						md_ = MODE::MOVING;	
						ptold = pt;
					}
				}
				else
					ret = DefWndScrollbarProc(d,message,wp,lp);
				
				if ( ret == 0 )								
					DoCapture();
				
				ret = 1;							
			}
			
		}
		break;
		case WM_MOUSEMOVE:
		{
			if ( md_ == MODE::MOVING )
			{
				FPointF pt = mat_.DPtoLP(lp);

				rc_.Offset( pt.x - ptold.x, pt.y - ptold.y );
				ptold = pt;


				ret = 1;
				d->redraw();
			}
			else
			{
				FPointF pt = mat_.DPtoLP(lp);
				if ( rc_.PtInRect(pt) )
				{
					ret = DefWndScrollbarProc(d,message,wp,lp);
					
				}
				
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			if ( md_ == MODE::MOVING )
			{
				md_ = MODE::NONE;	
				d->redraw();
			}
			else
			{
				FPointF pt = mat_.DPtoLP(lp);
				if ( rc_.PtInRect(pt) )
				{
					ret = DefWndScrollbarProc(d,message,wp,lp);
				}
				
			}

			ret = 1;

		}
		break;
		case WM_D2D_COMBOBOX_CHANGED:
		{
			D2DCombobox::wparam* xwp = (D2DCombobox::wparam*)wp;

			int a = xwp->cmb->SelectedIdx();

			int b = 0;
		}

		break;
		case WM_D2D_CLOSE_WINDOW:
		{			
			if ( wp == (INT_PTR)this )
			{
				this->DestroyControl();
				ret = 1;
			}
		}
		break;
		case WM_MOUSEWHEEL:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lp));

			//if ( rc_.PtInRect( pt ))		
			{
				ret = DefWndScrollbarProc(d,message,wp,lp);
			}
		}
		break;
		case WM_SIZE:
		{						
			

			if ( stat_ & AUTOSIZE )
			{
				FRectF rc = parent_control_->GetRect().GetContentRect();
				rc_ = rc.ZeroRect();
			}				

			auto rc = rc_.GetContentRectZero();

			FRectFBoxModel xrc = VScrollbarRect(rc);
			Vscbar_->SetRect(xrc);

			xrc = HScrollbarRect(rc);
			Hscbar_->SetRect(xrc);

			ret = D2DControls::DefWndProc(d,message,wp,lp);
		}
		break;

		default :
			ret = D2DControls::DefWndProc(d,message,wp,lp);
	}

	return ret;
}

bool D2DChildFrameEX::TB_MouseWindowResize( FMODE mode, FRectF rc, FPointF pt )
{
	if ( FMODE::TRY == mode )
	{
		return ( pt.x < rc.left + 10 && pt.y < rc.bottom );
	}
	else if ( FMODE::DO == mode )
	{
		if (wmd_ == WINDOWMODE::NORMAL)
		{
			prv_rc_ = rc_;
			FRectF xrc = rc_;

			auto rca = parent_control_->GetRect();

			xrc.SetSize( rca.Size()); 

			drift_ = std::make_shared<InfoDrift>();

			drift_->dstRect = xrc;
					


			wmd_ = WINDOWMODE::MAXMIZE;

		}
		else if (wmd_ == WINDOWMODE::MAXMIZE)
		{
			FRectF xrc = rc_;
			xrc.SetSize( prv_rc_.GetSize());
			prv_rc_ = xrc;

			drift_ = std::make_shared<InfoDrift>();
			drift_->dstRect = xrc;
			
			wmd_ = WINDOWMODE::NORMAL;
		}
	}
	return false;
}
bool D2DChildFrameEX::TB_DlgWindowProperty( FMODE mode, FRectF rc, FPointF pt )
{
	if ( FMODE::TRY == mode )
	{
		return ( rc.left + 10 < pt.x && pt.x < rc.left + 20 && pt.y < rc.bottom );
	}
	else if ( FMODE::DO == mode )
	{
		SlideMenuItem s;
		s.sender = this;

		//D2DDebugPlaceHolder* x = new D2DDebugPlaceHolder( ColorPallet_1[1]);
		
		D2DList* x = new D2DList();
		x->Create(parent_, this, FRectF(0,0,500,200 ), 0,L"noname" );
		s.item = x;

		parent_->SendMessage( WM_D2D_OPEN_SLIDE_MENU_EX, (INT_PTR)&s, nullptr );		
	}
	return false;
}
bool D2DChildFrameEX::TB_WindowClose( FMODE mode, FRectF rc, FPointF pt )
{
	if ( FMODE::TRY == mode )
	{
		return ( rc.left + 20 < pt.x && pt.x < rc.left + 30 && pt.y < rc.bottom );
	}
	else if ( FMODE::DO == mode )
	{
		if (wmd_ == WINDOWMODE::NORMAL || wmd_ == WINDOWMODE::MAXMIZE )
		{
			prv_rc_ = rc_;
			FRectF xrc = rc_;
			xrc.SetSize( 50,50 );

			drift_ = std::make_shared<InfoDrift>();

			drift_->dstRect = xrc;

			wmd_ = WINDOWMODE::MINIMIZE;

			drift_->completed = [this]()
			{
				parent_->PostMessage( WM_D2D_CLOSE_WINDOW, (INT_PTR)this, nullptr );

				//this->DestroyControl();　WM_PAINT内ではループをRET=1にできなのでエラーになる。POSTMESSAGEで後で処理する。
			};
		}		
	}
	return false;
}

void D2DChildFrameEX::ShowScrollbar( SCROLLBAR_TYP typ, bool visible )
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

int D2DChildFrameEX::DefWndScrollbarProc(D2DWindow* d, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam)
{
	int ret = Vscbar_->WndProc(d,message,wParam,lParam);
	if ( ret == 0 )
		ret = Hscbar_->WndProc(d,message,wParam,lParam);

	if ( ret == 0 )
		ret = D2DControls::DefWndProc(d,message,wParam,lParam);

	return ret;
}


void D2DChildFrameEX::SetCanvasSize( float cx, float cy )
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

void D2DChildFrameEX::UpdateScrollbar(D2DScrollbar* bar)
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