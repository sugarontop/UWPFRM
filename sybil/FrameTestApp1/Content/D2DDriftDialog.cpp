#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DDriftDialog.h"
#include "sybil.h"
#include "D2DCommon.h"

using namespace V4;
using namespace sybil;



D2DDriftDialog::D2DDriftDialog()
{

}
void D2DDriftDialog::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, local_id);

	default_v_ = [](D2DDriftDialog* x, D2DContext&cxt)
	{
		FRectF rcb = x->rc_.GetBorderRect();

		cxt.cxt->FillRectangle( rcb, cxt.ltgray );
		

	
	};

	md_ = MODE::MIN;
	cc_ = nullptr;
	orgRect_ = rc_;

	if ( OnCreate_ )
		OnCreate_(this);

}
int D2DDriftDialog::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( (stat_ & STAT::VISIBLE )== 0 ) 
		return 0;

	int ret = 0;
	bool bl = false;
	
	switch( message )
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());

			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();

			FRectF rcb = rc_.GetBorderRect();
															
			FRectF rca = rc_.GetContentBorderBase(); 

			if ( bfresize_ || cc_ !=nullptr )
			{
				FRectF rcz = dstRect_;
				FRectF rc;
				cc_ = sybil::DrawDriftRect( cc_, &rc, cxt.tickcount_, rc_, rcz, 500 );
				if ( cc_ == nullptr )
				{					
					bfresize_ = false;
					
					md_ = ( md_ == MODE::MAX2MIN ?  MODE::MIN :  MODE::MAX );

					if ( md_ == MODE::MIN )
					{
						_ASSERT ( ParentControl()->GetCapture() == this );
						
						if ( rc.Size().width == 0 || rc.Size().height == 0 )
							stat_ |=  ~STAT::VISIBLE;

						for( auto& it : controls_)
							it->Hide();

						ParentControl()->ReleaseCapture();

						OnClosed();
					}
				}

				rc_ = rc;

				default_v_(this, cxt );
							
				d->redraw();

				

			}
			else			
			{
				default_v_(this, cxt );				
			}
			

			mat.Offset(rcb.left, rcb.top);
			D2DRectFilter rcf( cxt, rcb.ZeroRect());
			D2DControls::DefPaintWndProc(d,message,wp,lp);

			mat.PopTransform();
			return 0;
		}
		break; 
		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			if ( parent_control_->GetCapture() == this  )
			{
				switch( arg->VirtualKey )
				{
					case Windows::System::VirtualKey::Escape:
					{						
						md_ = MODE::MAX2MIN;

						SetDriftRect( orgRect_ );
						
						ret = 1;
					}
				}
				if ( ret == 0 )
				{
					ret = D2DControls::DefWndProc(d,message,wp,lp);
					ret = 1;
				}

			}
			bl = true;
		}
		break;
		case WM_KEYUP:
		{
			if ( parent_control_->GetCapture() == this  )
			{
				ret = D2DControls::DefWndProc(d,message,wp,lp);
				ret = 1;
			}
			bl = true;
		}
		break;
		case WM_LBUTTONDOWN :		
		case WM_LBUTTONUP :
		case WM_MOUSEMOVE :		
		{
			FPointF pt = mat_.DPtoLP(lp);
			if ( rc_.PtInRect(pt ))
				ret = D2DControls::DefWndProc(d,message,wp,lp);

			if ( parent_control_->GetCapture() == this  )
			{		
				ret = 1;
			}

			bl = true;
		}
		break;		
		/*case WM_SIZE :
		{
			Windows::UI::Core::WindowSizeChangedEventArgs^ args = (Windows::UI::Core::WindowSizeChangedEventArgs^)lp;

			rc_.SetHeight( args->Size.Height );

			
			auto sz = rc_.Size();
			rc_.right =	args->Size.Width;
			rc_.left = rc_.right - sz.width;
		}
		break;*/
	}

	if ( ret == 0 && bl == false)
		ret = D2DControls::DefWndProc(d,message,wp,lp);
		

	return ret;

}
void D2DDriftDialog::SetDriftRect( const FRectF& rc )
{
	if  ( dstRect_ == rc ) return;

	dstRect_ = rc;
	bfresize_ = true;	
	parent_->redraw();
	stat_ &= STAT::VISIBLE;

	for( auto& it : controls_)
	{
		if ( md_ == MODE::MIN2MAX ||  md_ == MODE::MIN )
		{
			it->SetRect(rc.ZeroRect() );
		}
		
		it->Visible();
	}

	// captureしていたら縮小へリサイズ、していない場合 captureして最大化へ
	md_ = ( ParentControl()->GetCapture() == this ? MODE::MAX2MIN : MODE::MIN2MAX );

	if ( md_ == MODE::MIN2MAX )
	{
		ParentControl()->SetCapture(this );
	}
}

void D2DDriftDialog::OnClosed()
{
	// ParentExchangeされたobjを削除

	controls_.clear();

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

D2DDebugPlaceHolder::D2DDebugPlaceHolder():backclr_(ColorF(D2RGB(192,192,192)))
{
	
}
D2DDebugPlaceHolder::D2DDebugPlaceHolder(ColorF bk):backclr_(bk)
{
	
}
D2DDebugPlaceHolder::~D2DDebugPlaceHolder()
{
	TRACE( L"~D2DDebugPlaceHolder %x\n", this );
}
void D2DDebugPlaceHolder::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, local_id);
}
int D2DDebugPlaceHolder::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	int ret = 0;

	if ( !(stat_ & VISIBLE) )
		return ret;

	bool bl = true;

	switch( message )
	{

		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());
			
			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();

			FRectF rcb = rc_.GetBorderRect();
			mat.Offset(rcb.left, rcb.top); // border rect基準
						
						
			//FRectF rca = rc_.GetContentBorderBase(); 
			//cxt.cxt->FillRectangle( rca.ZeroRect(), cxt.halftoneRed );
			DrawDebugInfo( this, cxt );

			 D2DControls::DefPaintWndProc(d,message,wp,lp);
			 return 0;

		}
		break;
		case WM_MOUSEMOVE :
		{
			mouse_pt_ = FPointF(lp);

			auto str = V4::Format( L"mouse= %4.1f, %4.1f", mouse_pt_.x, mouse_pt_.y );

			layout1_.Release();
			CreateSingleTextLayout( parent_->cxt()->cxtt.wfactory, str.c_str(), str.length(), parent_->cxt()->cxtt.textformat, &layout1_ );
			ret = 1;

			d->redraw();
		}
		break;
	}

	if ( ret == 0 && bl )
		ret = D2DControls::DefWndProc(d,message,wp,lp);
	return ret;


}
void D2DDebugPlaceHolder::DrawDebugInfo( D2DDebugPlaceHolder* s, D2DContext& cxt )
{	
	auto rc = s->GetRect().ZeroRect();

	ComPTR<ID2D1SolidColorBrush> br;
	cxt.cxt->CreateSolidColorBrush(s->backclr_, &br );

	cxt.cxt->FillRectangle( rc, br );

	auto rc2 = rc;
	rc2.InflateRect( -5,-2 );

	cxt.cxt->DrawRectangle( rc2, cxt.black );

	std::wstring str = L"D2DDebugPlaceHolder";
	cxt.cxt->DrawText( (LPCWSTR)str.c_str(), str.length(), cxt.cxtt.textformat, rc2, cxt.white );

	rc2.Offset( 0, 30 );
	str = V4::Format( L"instance=%x, rc= %4.1f, %4.1f, %4.1f, %4.1f", (DWORD)s, rc.left, rc.top, rc.right, rc.bottom );

	cxt.cxt->DrawText( (LPCWSTR)str.c_str(), str.length(), cxt.cxtt.textformat, rc2, cxt.white );

	rc2.top += 30;

	if ( s->layout1_ )
		DrawTextLayoutCenter( cxt.cxt, rc2, s->layout1_,cxt.white );

}