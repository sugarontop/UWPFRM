#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DDriftDialog.h"
#include "D2DDevelop.h"
#include "sybil.h"


using namespace V4;

#define BARWIDTH 18.0f
#define SCBAR(x) ((D2DScrollbar*)x.get())
#define TITLEBAR_HEIGHT 24.0f

void BlackBackYellow( D2DContext& cxt, D2D1_RECT_F& rc )
{
	FRectF rcc(rc);

	cxt.cxt->FillRectangle(rc, cxt.yellow );
	rcc.InflateRect(-1,-1);
	rcc.right-=BARWIDTH;
	rcc.bottom-=BARWIDTH;
	cxt.cxt->FillRectangle(rcc, cxt.black );
}
void BlackBack( D2DContext& cxt, D2D1_RECT_F& rc )
{
	cxt.cxt->FillRectangle(rc, cxt.black );
}
FRectF RBRectF(FRectF& rc, float wh)
{
	return FRectF( rc.right-wh,rc.bottom-wh,rc.right+wh,rc.bottom+wh);

}

void D2DChildFrame2::Create(D2DControls* pacontrol, const FRectFBoxModel& rc,int stat,LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol,rc,stat, name, local_id);
	prrc_ = rc;
	scale_ = 1.0f;
	// V Scrollbar///////////////
	FRectFBoxModel xrc = VScrollbarRect(rc.GetContentRectZero());

	auto wfac = parent_->cxt()->cxtt.wfactory;
	auto tf = parent_->cxt()->cxtt.textformat;
	sybil::CreateSingleTextLayout( wfac, name_.c_str(), name_.length(), tf, &title_ );


	D2DScrollbar* Vscbar = new D2DScrollbar();
	Vscbar->Create(this,xrc,VISIBLE,NONAME );

	Vscbar_ = controls_[0];
	controls_.clear();

	// H Scrollbar///////////////
	xrc = HScrollbarRect(rc.GetContentRectZero());

	auto Hscbar = new D2DScrollbar();
	Hscbar->Create(this,xrc,VISIBLE,NONAME );
	Hscbar_ = controls_[0];
	controls_.clear();

	scrollbar_off_.height = 0;
	scrollbar_off_.width = 0;

	SCBAR(Vscbar_)->Hide();
	SCBAR(Hscbar_)->Hide();

	titlebar_enable_ = true;
	back_ground_ = BlackBack;
}
int D2DChildFrame2::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{	
	if (IsHide() && !IsImportantMsg(message) ) 
		return 0;

	int ret = 0;
	
	
	switch( message )
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());


			DrawDefault(cxt,d,wp);
			
			
			return 0;
		}
		break;
		case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		{	
			if ( wmsize_ )
			{
				rc_ = wmsize_(this);				
			}
			else
			{
				if (stat_ & AUTOSIZE)
				{
					FRectF rc = parent_control_->GetRect().GetContentRect();
					rc_ = rc.ZeroRect();

					float h = TITLEBAR_HEIGHT; // タイトルバーの高さ
					rc_.top -= h;
					rc.bottom += h;
				}				
			}

			// scrollbarの設定
			Resize();

			D2DControls::DefPaintWndProc(d,message,wp,lp);
			return 0;
		}
		break;

		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(lp);

			if ( RBRectF(rc_,10.0f).PtInRect(pt))
			{
				md_ = MODE::RESIZE;

				GetParentControl()->SetCapture(this);

				D2DMainWindow::SetCursor(CURSOR_SizeNorthwestSoutheast);

				ret = 1;
				return ret;



			}
			else if ( rc_.PtInRect(pt ))
			{
				md_ = MODE::NONE;

				auto rc_title_bar = rc_;
				rc_title_bar.bottom = rc_title_bar.top + (titlebar_enable_ ? TITLEBAR_HEIGHT : 0);
				
				if ( rc_title_bar.PtInRect(pt))			
				{
					md_ = MODE::MOVING;

					GetParentControl()->SetCapture(this);

					ret = 1;
				}
				else if ( rc_.right- BARWIDTH < pt.x || rc_.bottom- BARWIDTH < pt.y )
				{
					md_ = MODE::SCROLLBAR;				
					GetParentControl()->SetCapture(this);
					ret = InnerDefWndScrollbarProc(d,message,wp,lp);
				}


				if ( ret == 0 )
					ret = D2DControls::DefWndProc(d,message,wp,lp);


				ret = 1;
				return ret;
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			FPointF pt(lp);
			FPointF pt3 = mat_.DPtoLP(lp);

			D2DMainWindow::SetCursor(CURSOR_ARROW);

			if ( md_ == MODE::RESIZE && GetParentControl()->GetCapture() == this )
			{
				WParameterMouse* prvm = (WParameterMouse*)wp;
				
			
				rc_.right += pt.x-prvm->move_ptprv.x;
				rc_.bottom += pt.y-prvm->move_ptprv.y;
				ret = 1;
				d->redraw();

				

				//// scrollbarの設定
				auto xrc = VScrollbarRect(rc_.ZeroRect());
				Vscbar_->SetRect(xrc);

				xrc = HScrollbarRect(rc_.ZeroRect());
				Hscbar_->SetRect(xrc);

				
				auto sz = rc_.Size();
				for(auto& it : controls_ )
					it->WndProc(parent_,WM_D2D_RESIZE,(INT_PTR)&sz,nullptr);
			}			
			else if ( md_ == MODE::MOVING && GetParentControl()->GetCapture() == this && titlebar_enable_)
			{
				WParameterMouse* prvm = (WParameterMouse*)wp;
						
				rc_.Offset(pt.x-prvm->move_ptprv.x, pt.y-prvm->move_ptprv.y);

				ret = 1;
				d->redraw();


			}
			else if ( md_ == MODE::SCROLLBAR )
				ret = InnerDefWndScrollbarProc(d,message,wp,lp);
			else if ( md_ == MODE::NONE && RBRectF(rc_,10.0f).PtInRect(pt3) )
			{
				D2DMainWindow::SetCursor(CURSOR_SizeNorthwestSoutheast);

				ret = 1;
				d->redraw();

			}
		}
		break;
		case WM_LBUTTONUP:
		{			
			if ( GetParentControl()->GetCapture() == this && md_ == MODE::RESIZE)
			{
				GetParentControl()->ReleaseCapture();
				D2DMainWindow::SetCursor(CURSOR_ARROW);

				ret = 1;
			}
			else if ( GetParentControl()->GetCapture() == this )
			{
				GetParentControl()->ReleaseCapture();


				WParameter wpm;
				wpm.sender = this;
				wpm.target = this;

				d->SendMessage(WM_D2D_TAB_ACTIVE,(INT_PTR)&wpm, nullptr);
				ret = 1;
			}


			if ( md_ == MODE::SCROLLBAR )
			{				
				ret = InnerDefWndScrollbarProc(d,message,wp,lp);
			}
	
			md_ = MODE::NONE;
		}
		break;
		case WM_LBUTTONDBLCLK:
		{
			FPointF pt = mat_.DPtoLP(lp);
			if ( rc_.PtInRect(pt ))
			{
				md_ = MODE::NONE;

				auto rc_title_bar = rc_;
				rc_title_bar.bottom = rc_title_bar.top + TITLEBAR_HEIGHT;
				
				if ( rc_title_bar.PtInRect(pt))		
				{
					TitlebarDblclick();
					ret = 1;
				}
			}			
		}
		break;
		case WM_D2D_TAB_DETACH:
		{
			titlebar_enable_ = true;			
			Resize();
			d->redraw();
		}
		break;
		case WM_D2D_TAB_ACTIVE:
		{
			WParameter* wpm = (WParameter*)wp;

			active_ = (this == wpm->sender);

			//back_ground_ = (active_? BlackBackYellow : BlackBack);
		}
		break;
		case WM_D2D_VSCROLLBAR_SHOW:
		{
			WParameter* wps = (WParameter*)wp;
			if ( wps->target == this )
			{
				if ( wps->no == 1 )
					SCBAR(Vscbar_)->Visible();
				else
					SCBAR(Vscbar_)->Hide();
				ret = 1;
			}
		}
		break;
		case WM_D2D_HSCROLLBAR_SHOW:
		{
			WParameter* wps = (WParameter*)wp;
			if ( wps->target == this )
			{
				if ( wps->no == 1 )
					SCBAR(Hscbar_)->Visible();
				else
					SCBAR(Hscbar_)->Hide();
				ret = 1;
			}
		}
		break;

	}

	if ( WM_D2D_USERCD <= message )
	{
		
		if ( Extention_.find(message) != Extention_.end())
			ret = Extention_[message](message,wp,lp);

	}

	if ( ret == 0 )
		ret = D2DControls::DefWndProc(d,message,wp,lp);

	return ret;
}
bool D2DChildFrame2::TitlebarDblclick()
{
	titlebar_enable_ = false;

	prrc_ = rc_;

	// 全体にWM_SIZEを投げる
	auto a = dynamic_cast<D2DMainWindow*>(parent_);	
	a->ReSize();

	return true;
}


// 
void D2DChildFrame2::Resize()
{
	D2DTabControls* w = nullptr;
	D2DControls* p = this;
	while( w == nullptr && p )
	{
		p = p->GetParentControl();
		w = dynamic_cast<D2DTabControls*>(p);
	}

	if ( w ) 
	{
		auto rc = rc_;

		if (titlebar_enable_ == false )
			rc = w->GetContentRect();
		else
			rc = prrc_;

		SetRect(rc);
		
		auto xrc = VScrollbarRect(rc.ZeroRect());
		Vscbar_->SetRect(xrc);

		xrc = HScrollbarRect(rc.ZeroRect());
		Hscbar_->SetRect(xrc);
	}
}




void D2DChildFrame2::DrawDefault(D2DContext& cxt, D2DWindow* d, INT_PTR wp)
{
	D2DMatrix mat(cxt);
	mat_ = mat.PushTransform();
	FRectF rcb = rc_.GetBorderRect();

	auto rcb1 = rcb.ZeroRect();
	float offh = 0;
	if ( titlebar_enable_ )	
	{
		mat.Offset(rcb.left, rcb.top);			
		rcb1.top += DrawTitle( cxt, rcb );	
		offh = TITLEBAR_HEIGHT;
	}

	D2DRectFilter f(cxt, rcb1 ); 

	back_ground_(cxt, rcb1);

	mat.PushTransform();
	{
		mat.Scale(scale_,scale_);
		mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height+offh );

		DefPaintWndProc(d,WM_PAINT,wp,nullptr); 
	}
	mat.PopTransform();
			
	Vscbar_->WndProc(d,WM_PAINT,0,nullptr);
	Hscbar_->WndProc(d,WM_PAINT,0,nullptr);

	mat.PopTransform();
}
FRectFBoxModel D2DChildFrame2::VScrollbarRect( const FRectFBoxModel& rc )
{
	_ASSERT(rc.left == 0&&rc.top == 0);
	FRectFBoxModel xrc(rc);
	xrc.left = xrc.right - BARWIDTH;
	xrc.top += (titlebar_enable_ ? TITLEBAR_HEIGHT : 0);
	xrc.bottom -= BARWIDTH;

	return xrc;
}
FRectFBoxModel D2DChildFrame2::HScrollbarRect( const FRectFBoxModel& rc )
{
	_ASSERT(rc.left == 0&&rc.top == 0);
	FRectFBoxModel xrc(rc);
	xrc.top = xrc.bottom - BARWIDTH;
	xrc.right -= BARWIDTH;

	return xrc;
}

float D2DChildFrame2::DrawTitle(D2DContext& cxt, const FRectF& rc )
{
	FRectF rc1 = rc.ZeroRect();

	cxt.cxt->DrawRectangle( rc1, cxt.black );
	cxt.cxt->FillRectangle( rc1, cxt.white );
	rc1.bottom = rc1.top + TITLEBAR_HEIGHT;

	if ( IsCaptured() || active_ )
	{
		cxt.cxt->FillRectangle( rc1,  cxt.bluegray );
		sybil::DrawTextLayoutCenter( cxt.cxt, rc1, title_,  cxt.white  ); 
	}
	else
	{
		cxt.cxt->FillRectangle( rc1,  cxt.gray );
		sybil::DrawTextLayoutCenter( cxt.cxt, rc1, title_,  cxt.black  ); 
	}

	return TITLEBAR_HEIGHT;
}

void D2DChildFrame2::SetCanvasSize( float cx, float cy )
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

	SCBAR(Vscbar_)->Visible();
	SCBAR(Hscbar_)->Visible();

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



int D2DChildFrame2::InnerDefWndScrollbarProc(D2DWindow* d, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam)
{
	int ret = Vscbar_->WndProc(d,message,wParam,lParam);
	if ( ret == 0 )
		ret = Hscbar_->WndProc(d,message,wParam,lParam);
	return ret;
}


void D2DChildFrame2::UpdateScrollbar(D2DScrollbar* bar)
{
	auto& info = bar->Info();
	
	if ( info.bVertical )
		scrollbar_off_.height = info.position / info.thumb_step_c;
	else
		scrollbar_off_.width = info.position / info.thumb_step_c;
}

