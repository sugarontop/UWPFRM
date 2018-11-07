#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DDriftDialog.h"
#include "D2DDevelop.h"
#include "sybil.h"


using namespace V4;

static D2DColor ColorPallet_1[] = { D2DColor(0x59202c),D2DColor(0x7f2c3e),D2DColor(0xa6385e),D2DColor(0x898989),D2DColor(0xa2a2a2) };

#define DEEPCOLOR	DRGB(ColorPallet_1[0])
#define CENTERCOLOR	DRGB(ColorPallet_1[1])
#define LIGHTCOLOR	DRGB(ColorPallet_1[2])
#define TEXTCOLOR	DRGB(ColorPallet_1[3])
#define LIGHTTEXTCOLOR	DRGB(ColorPallet_1[4])

#define BARWIDTH 18.0f

#define SCBAR(x) ((D2DScrollbar*)x.get())

#define TITLEBAR_HEIGHT 24.0f

FRectFBoxModel D2DChildFrame::VScrollbarRect( const FRectFBoxModel& rc )
{
	FRectFBoxModel xrc(rc);
	xrc.left = xrc.right - BARWIDTH;
	xrc.top += TITLEBAR_HEIGHT;
	xrc.bottom -= BARWIDTH;

	return xrc;
}
FRectFBoxModel D2DChildFrame::HScrollbarRect( const FRectFBoxModel& rc )
{
	FRectFBoxModel xrc(rc);
	xrc.top = xrc.bottom - BARWIDTH;
	xrc.right -= BARWIDTH;

	return xrc;
}

void BlackBack( D2DContext& cxt, D2D1_RECT_F& rc )
{
	cxt.cxt->FillRectangle(rc, cxt.black);
}

void D2DChildFrame::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, WINSTYLE ws,LPCWSTR name, int local_id )
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, local_id);
	md_ = MODE::NONE;
	wmd_ = WINDOWMODE::NORMAL;
	auto wfac = parent_->cxt()->cxtt.wfactory;
	auto tf = parent_->cxt()->cxtt.textformat;
	scale_ = 1.0f;
	prv_rc_ = rc;

	back_ground_ = BlackBack;

	sybil::CreateSingleTextLayout( wfac, name_.c_str(), name_.length(), tf, &title_ );

	// V Scrollbar///////////////
	FRectFBoxModel xrc = VScrollbarRect(rc.GetContentRectZero());

	D2DScrollbar* Vscbar = new D2DScrollbar();
	Vscbar->Create(parent,this,xrc,VISIBLE,NONAME );

	Vscbar_ = controls_[0];
	controls_.clear();

	// H Scrollbar///////////////
	xrc = HScrollbarRect(rc.GetContentRectZero());

	auto Hscbar = new D2DScrollbar();
	Hscbar->Create(parent,this,xrc,VISIBLE,NONAME );
	Hscbar_ = controls_[0];
	controls_.clear();

	scrollbar_off_.height = 0;
	scrollbar_off_.width = 0;

	SCBAR(Vscbar_)->Hide();
	SCBAR(Hscbar_)->Hide();
}
void D2DChildFrame::OnReleaseCapture(int layer)
{
	D2DControl::OnReleaseCapture(layer);

	title_.Release();		
	sybil::CreateSingleTextLayout( GetDWFactory(), name_.c_str(), name_.length(), GetTextFormat(), &title_ );
}
void D2DChildFrame::OnSetCapture(int layer)
{
	D2DControl::OnSetCapture(layer);

	title_.Release();
	sybil::CreateSingleTextLayout( GetDWFactory(), L"Active",6, GetTextFormat(), &title_ );
}


void D2DChildFrame::DrawDriftRect(D2DWindow* d, D2DContext& cxt)
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
void D2DChildFrame::DrawTitle(D2DContext& cxt, const FRectF& rc )
{
	FRectF rc1 = rc.ZeroRect();

	cxt.cxt->DrawRectangle( rc1, cxt.black );
	cxt.cxt->FillRectangle( rc1, cxt.white );
	rc1.bottom = rc1.top + TITLEBAR_HEIGHT;

	if ( IsCaptured() )
	{
		cxt.cxt->FillRectangle( rc1,  cxt.bluegray );
		sybil::DrawTextLayoutCenter( cxt.cxt, rc1, title_,  cxt.white  ); 
	}
	else
	{
		cxt.cxt->FillRectangle( rc1,  cxt.gray );
		sybil::DrawTextLayoutCenter( cxt.cxt, rc1, title_,  cxt.black  ); 
	}
/*
	// left side button
	{
		FRectF rcb(0,0,10,TITLEBAR_HEIGHT);
		cxt.cxt->FillRectangle( rcb, LIGHTCOLOR ); // maxsize
		rcb.Offset(10,0);
		cxt.cxt->FillRectangle( rcb, CENTERCOLOR ); // property
		rcb.Offset(10,0);
		cxt.cxt->FillRectangle( rcb, DRGB(D2DColor(0x59202c)) ); // close

		rcb.Offset(10,0);
		cxt.cxt->FillRectangle( rcb, DRGB(D2DColor(0x59992c)) ); // minimize
		rcb.Offset(10,0);
		cxt.cxt->FillRectangle( rcb, LIGHTCOLOR ); // MDI detach


	}
*/
}





void D2DChildFrame::SetScale( float scale )
{
	scale_ = scale;
	parent_->redraw();
}
void D2DChildFrame::DrawDefault(D2DContext& cxt, D2DWindow* d, INT_PTR wp)
{
	D2DMatrix mat(cxt);
	mat_ = mat.PushTransform();
	FRectF rcb = rc_.GetBorderRect();

	mat.Offset(rcb.left, rcb.top);			
	
	DrawTitle( cxt, rcb );
	auto rcb1 = rcb.ZeroRect();
	rcb1.top += TITLEBAR_HEIGHT;
	D2DRectFilter f(cxt, rcb1 ); 

	back_ground_(cxt, rcb1);

	mat.PushTransform();
	{
		mat.Scale(scale_,scale_);
		mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height+TITLEBAR_HEIGHT );

		DefPaintWndProc(d,WM_PAINT,wp,nullptr); 
	}
	mat.PopTransform();
			
	Vscbar_->WndProc(d,WM_PAINT,0,nullptr);
	Hscbar_->WndProc(d,WM_PAINT,0,nullptr);

	mat.PopTransform();
}
void D2DChildFrame::DrawMinimize(D2DContext& cxt, D2DWindow* d, INT_PTR wp)
{
	D2DMatrix mat(cxt);
	mat_ = mat.PushTransform();
	FRectF rcb = rc_.GetBorderRect();
	mat.Offset(rcb.left, rcb.top);			
	
	DrawTitle( cxt, rcb );
	auto rcb1 = rcb.ZeroRect();
	rcb1.top += TITLEBAR_HEIGHT;
	D2DRectFilter f(cxt, rcb1 ); 
			
	mat.PushTransform();
	{
		mat.Scale(scale_,scale_);
	

		//DefPaintWndProc(d,WM_PAINT,wp,nullptr); 
		cxt.cxt->FillRectangle( rcb1, cxt.bluegray );
	}
	mat.PopTransform();
			
	

	mat.PopTransform();
}

int D2DChildFrame::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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


			if ( wmd_ == WINDOWMODE::MINIMIZE )
				DrawMinimize(cxt, d, wp);
			else
				DrawDefault(cxt, d, wp);
			
			
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
				rc_title_bar.bottom = rc_title_bar.top + TITLEBAR_HEIGHT;
				
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
					{
						TB_WindowClose(FMODE::DO,rc_title_bar, pt );
						ret = 1;
					}
					else if ( TB_MinimizeWindow(FMODE::TRY,rc_title_bar, pt ) )
					{
						TB_MinimizeWindow(FMODE::DO,rc_title_bar, pt );
						ret = 1;
					}
					else if ( TB_MDIDetach(FMODE::TRY,rc_title_bar, pt ) )
					{
						TB_MDIDetach(FMODE::DO,rc_title_bar, pt );
						ret = 1;
					}
					else 
					{
						md_ = MODE::MOVING;	
						ptold = pt;
						ret = 1;
						DoCapture();
					}
				}

				if ( ret == 0 )
				{
					auto rc_rightbottom = rc_;
					rc_rightbottom.left = rc_rightbottom.right - 10;
					rc_rightbottom.top = rc_rightbottom.bottom - 10;
					if ( rc_rightbottom.PtInRect(pt))
					{
						md_ = MODE::RESIZE;	
						ptold = pt;
						ret = 1;
					}
				}

				if ( ret == 0 )
					ret = InnerDefWndScrollbarProc(d,message,wp,lp);
				
		
				ret = 1;							
			}
			else
			{
				if ( this->GetCapture() == this )
					ReleaseCapture(this);
			}
			
		}
		break;
		case WM_MOUSEMOVE:
		{
			if ( md_ == MODE::MOVING )
			{
				D2DMainWindow::SetCursor(CURSOR_ARROW);

				FPointF pt = mat_.DPtoLP(lp);

				rc_.Offset( pt.x - ptold.x, pt.y - ptold.y );
				ptold = pt;

				ret = 1;
				d->redraw();
			}
			else if ( md_ == MODE::RESIZE )
			{
				/*D2DMainWindow::SetCursor(CURSOR_SizeNorthwestSoutheast);

				FPointF pt = mat_.DPtoLP(lp);

				rc_.right += pt.x - ptold.x;
				rc_.bottom += pt.y - ptold.y;
				ptold = pt;


				rc_.right = max(rc_.left+5, rc_.right );
				rc_.bottom = max(rc_.top+5, rc_.bottom );


				ret = 1;
				d->redraw();
				wmd_ = WINDOWMODE::NORMAL;

				WndProc( d, WM_SIZE, 0,nullptr);*/
			}
			else
			{
				D2DMainWindow::SetCursor(CURSOR_ARROW);

				FPointF pt = mat_.DPtoLP(lp);
				if ( rc_.PtInRect(pt) || IsCaptured() )
				{
					ret = InnerDefWndScrollbarProc(d,message,wp,lp);					
				}				
			}

			if ( ret == 0 && IsCaptured())
				ret = 1;
		}
		break;
		case WM_LBUTTONUP:
		{
			if ( md_ == MODE::MOVING || md_ == MODE::RESIZE )
			{
				md_ = MODE::NONE;	
				D2DMainWindow::SetCursor(CURSOR_ARROW);
				d->redraw();
				ret = 1;
			}
			else
			{
				ret = InnerDefWndScrollbarProc(d,message,wp,lp);
			}
		}
		break;

		case WM_LBUTTONDBLCLK:
		{
			FPointF pt = mat_.DPtoLP(lp);
			if ( rc_.PtInRect(pt ))
			{
				auto rc_title_bar = rc_;
				rc_title_bar.bottom = rc_title_bar.top + TITLEBAR_HEIGHT;
				
				if ( rc_title_bar.PtInRect(pt))		
				{

					TitlebarDblclick();

					ret = 1;

				}
			}
			
			if ( ret == 0 )
				ret = DefWndProc(d,message,wp,lp);
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
				if ( parent_control_->GetCapture() == this )
					parent_control_->DoCapture();
								
				DestroyControl();
				ret = 1;
			}
			else
				ret = InnerDefWndScrollbarProc(d,message,wp,lp);
		}
		break;
		case WM_MOUSEWHEEL:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lp));

			if ( rc_.PtInRect( pt ))		
			{
				ret = InnerDefWndScrollbarProc(d,message,wp,lp);
			}
			
		}
		break;
		case WM_SIZE:
		{						
			if (stat_ & AUTOSIZE)
			{
				FRectF rc = parent_control_->GetRect().GetContentRect();
				rc_ = rc.ZeroRect();

				float h = TITLEBAR_HEIGHT; // タイトルバーの高さ
				rc_.top -= h;
				rc.bottom += h;
			}				


			// scrollbarの設定
			auto rc = rc_.GetContentRectZero();

			auto xrc = VScrollbarRect(rc);
			Vscbar_->SetRect(xrc);

			xrc = HScrollbarRect(rc);
			Hscbar_->SetRect(xrc);

			D2DControls::DefWndProc(d,message,wp,lp);
		}
		break;

		case WM_D2D_PREVIOUS_STATE:
		{
			WParameter* wpr = (WParameter*)wp;
			if ( wpr && wpr->target == this )
			{
				rc_ = prv_rc_;
				stat_ &= ~AUTOSIZE;

				WndProc(d,WM_SIZE,0,nullptr);
				ret = 1;
				d->redraw();
			}
		}
		break;
		case WM_DESTROY:
		{
			Vscbar_->WndProc(d,WM_DESTROY,0,nullptr);
			Hscbar_->WndProc(d,WM_DESTROY,0,nullptr);
			D2DControls::DefWndProc(d,message,wp,lp);
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



bool D2DChildFrame::TB_MouseWindowResize( FMODE mode, FRectF rc, FPointF pt )
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
bool D2DChildFrame::TB_DlgWindowProperty( FMODE mode, FRectF rc, FPointF pt )
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
bool D2DChildFrame::TB_WindowClose( FMODE mode, FRectF rc, FPointF pt )
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

bool D2DChildFrame::TB_MinimizeWindow( FMODE mode, FRectF rc, FPointF pt )
{
	if ( FMODE::TRY == mode )
	{
		return ( rc.left + 30 < pt.x && pt.x < rc.left + 40 && pt.y < rc.bottom );
	}
	else if ( FMODE::DO == mode )
	{
		if (wmd_ == WINDOWMODE::NORMAL || wmd_ == WINDOWMODE::MAXMIZE )
		{
			prv_rc_ = rc_;
			FRectF xrc = rc_;
			xrc.SetSize( 150,150 );

			drift_ = std::make_shared<InfoDrift>();

			drift_->dstRect = xrc;

			wmd_ = WINDOWMODE::MINIMIZE;

			//drift_->completed = [this]()
			//{
			//	parent_->PostMessage( WM_D2D_CLOSE_WINDOW, (INT_PTR)this, nullptr );

			//	//this->DestroyControl();　WM_PAINT内ではループをRET=1にできなのでエラーになる。POSTMESSAGEで後で処理する。
			//};
		}		
	}
	return false;
}
bool D2DChildFrame::TB_MDIDetach( FMODE mode, FRectF rc, FPointF pt )
{
	if ( FMODE::TRY == mode )
	{
		return ( rc.left + 40 < pt.x && pt.x < rc.left + 50 && pt.y < rc.bottom );
	}
	else if ( FMODE::DO == mode )
	{
		if ( !mdi_prev_.kls.empty() )
			MDI_Docking( false,  this );

	}
	return false;
}
void D2DChildFrame::ShowScrollbar( SCROLLBAR_TYP typ, bool visible )
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

int D2DChildFrame::InnerDefWndScrollbarProc(D2DWindow* d, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam)
{
	int ret = Vscbar_->WndProc(d,message,wParam,lParam);
	if ( ret == 0 )
		ret = Hscbar_->WndProc(d,message,wParam,lParam);

	if ( ret == 0 )
		ret = D2DControls::DefWndProc(d,message,wParam,lParam);

	return ret;
}


void D2DChildFrame::SetCanvasSize( float cx, float cy )
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

void D2DChildFrame::UpdateScrollbar(D2DScrollbar* bar)
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

void D2DChildFrame::TitlebarDblclick()
{
	auto pc1 = GetParentControl();
	D2DChildFrame* k = nullptr;
	while( pc1 )
	{	
		auto pc21 = dynamic_cast<D2DChildFrame*>(pc1);
		if ( pc21 )
		{
			k = pc21;
			break;
		}
	
		pc1 = pc1->GetParentControl();
	}


	if ( k )
	{
		if ( parent_control_->GetCapture() == this )
			parent_control_->ReleaseCapture();

		MDI_Docking( true,  k );
	}
	else
	{		
		D2DMainWindow* w = dynamic_cast<D2DMainWindow*>(GetParentControl());

		if ( w )
			MDI_Docking( true,  w );
	}
}
void D2DChildFrame::MDI_Docking( bool IsDocking, D2DMainWindow* k )
{
	prv_rc_ = rc_;

	stat_  = stat_ | AUTOSIZE;

	WndProc( k, WM_SIZE, 0, nullptr );

}
void D2DChildFrame::MDI_Docking( bool IsDocking, D2DChildFrame* k )
{
	if ( IsDocking )
	{
		std::vector<std::shared_ptr<D2DControl>>& kcl = k->controls_;

		if ( !capture_.empty() )
			ReleaseCapture();

		capture_.clear();


		k->mdi_prev_.kls = kcl;
		k->mdi_prev_.hls = this->controls_;
		k->mdi_prev_.h = this;
		
		kcl.clear();

		for( auto& it : controls_ )
		{
			kcl.push_back(it);

			it->SetNewParentControl( k );			
		}
		
		this->Hide();
		controls_.clear();
	}
	else
	{
		_ASSERT( k == this );

		auto& hls = mdi_prev_.hls; 

		if ( !capture_.empty() )
			ReleaseCapture();

		capture_.clear();


		mdi_prev_.h->controls_ = hls;

		for( auto& it : hls )
		{			
			it->SetNewParentControl( mdi_prev_.h );	
		}

		controls_ = mdi_prev_.kls;

		mdi_prev_.h->Visible();





		mdi_prev_.hls.clear();
		mdi_prev_.kls.clear();
		mdi_prev_.h = nullptr;

	}

	
	
}