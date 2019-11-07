#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DSplitControls.h"
#include "sybil.h"
using namespace V4;

#define  DRGB ColorPtr

void D2DControlsView::Create( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, id);
}
int D2DControlsView::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( IsHide() && message != WM_SIZE && WM_D2D_INIT_UPDATE != message )
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
			
			//cxt.cxt->DrawRectangle( rcborder, br );
			cxt.cxt->FillRectangle( rcborder, DRGB(back_) );
			
			FRectF rc = rc_.GetContentRect();
			
			D2DRectFilter f(cxt, rc); 
			
			mat.Offset( rcborder.left, rcborder.top );		
			mat_ = mat; // 自座標(左上0,0)のmatrix
						
			{				
				DefPaintWndProc(d,message,wp,lp); 
			}

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		{
			FRectF rc = parent_control_->GetInnerRect(id_);
			rc_ = rc;

			
			D2DControls::DefWndProc(d,message,wp,lp);
		}
		break;

		default :
			ret = D2DControls::DefWndProc(d,message,wp,lp);
	}

	return ret;

}
///////////////////////////////////////////////////////////////////////////////////////////


void D2DSplitControls::Create( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, id);

	D2DControlsView* p1 = new D2DControlsView();
	D2DControlsView* p2 = new D2DControlsView();
	
	FRectF rc1(0,0,311,100);
	p1->Create(parent,this, rc1,VISIBLE,L"noname", 10 ); p1->SetBack(D2RGBA(0,0,255,255 ));
	FRectF rc2(312,0,1000,100);
	p2->Create(parent,this, rc2,VISIBLE,L"noname", 11 );p2->SetBack(D2RGBA(0,200,200,5));

	left_ = p1;
	right_ = p2;
}
int D2DSplitControls::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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
			
			//cxt.cxt->DrawRectangle( rcborder, br );
			//cxt.cxt->FillRectangle( rcborder, DRGB(back_) );
			
			FRectF rc = rc_.GetContentRect();
			D2DRectFilter f(cxt, rc );
			
			mat.Offset( rcborder.left, rcborder.top );		
			mat_ = mat; // 自座標(左上0,0)のmatrix
						
			{				
				DefPaintWndProc(d,message,wp,lp); 
			}

			mat.PopTransform();
			return 0;


		}
		break;
		case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		{
			rc_ = parent_control_->GetInnerRect(0); //GetRect().GetContentRect();
					
			D2DControls::DefWndProc(d,message,wp,lp);
		}
		break;

		default :
			ret = D2DControls::DefWndProc(d,message,wp,lp);
	}

	return ret;

}

FRectF D2DSplitControls::GetInnerRect(int idx ) 
{
	FRectF rc = parent_control_->GetRect().GetContentRect().ZeroRect();
	
	if ( idx == 10 )
		return FRectF(0,0, 330, rc_.Height());
	if ( idx == 11 )
		return FRectF(330,0, rc_.right, rc_.Height());

	return rc;


}

D2DControls* D2DSplitControls::GetInner(int idx)
{
	if ( idx == 10 )
		return left_; 
	else if ( idx == 11 )
		return right_; 
	return nullptr;
}
void D2DSplitControls::SetInner( D2DControls* newcontrols, bool bLeft )
{	
	_ASSERT(newcontrols);
	newcontrols->ParentExchange(this);

	if ( bLeft )
	{
		// 左側を交換
		for( auto it = controls_.begin(); it != controls_.end(); ++it )
		{
			if ( (*it).get() == left_ )
			{	
				controls_.erase(it);
				left_ = newcontrols;
				break;
			}			
		}		
	}
	else 
	{
		// 右側を交換
		for( auto it = controls_.begin(); it != controls_.end(); ++it )
		{
			if ( (*it).get() == right_ )
			{	
				controls_.erase(it);
				right_ = newcontrols;
				break;
			}			
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////

D2DSlideControls::D2DSlideControls()
{				
	ty_ = LEFT_RIGHT_SLIDE; 
	back_ = D2RGB(192,192,192);

}
void D2DSlideControls::Create( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, id);

	D2DControlsView* p1 = new D2DControlsView();
	D2DControlsView* p2 = new D2DControlsView();
	
	float hw = rc.Width();
	float hh = rc.Height();

	FRectF rc1(0,0,hw,hh);
	p1->Create(parent,this, rc1,VISIBLE,L"noname", 10 ); p1->SetBack(D2RGBA(0,0,255,255 ));
	FRectF rc2(0,0,FSizeF(hw,hh));
	p2->Create(parent,this, rc2,VISIBLE,L"noname", 11 );p2->SetBack(D2RGBA(0,200,0,255));


	FRectF rcb(10,100,FSizeF(100,26));
	D2DButton* bex = new D2DButton();
	bex->Create( parent, p2, rcb, VISIBLE, L"test", L"noname" );

	bex->OnClick_ = [parent,p2,pacontrol](D2DButton*b)
	{

		FRectF rc1( 100,100,FSizeF(80,50));
		
		D2DMessageBox::Show( parent, rc1, L"hoi", L"grp msgbox" );
	};





	left_ = p1;
	right_ = p2;

	p2->Hide();

	bSliding_ = false;
	cc_ = nullptr;
}
FRectF D2DSlideControls::GetInnerRect(int idx ) 
{
	FRectF rc = parent_control_->GetRect().GetContentRect().ZeroRect();
	
	if ( idx == 10 )
		return FRectF(0,0, rc_.Size());
	if ( idx == 11 )
		return FRectF(0,0, rc_.Size());

	return rc;


}
void D2DSlideControls::OnDrawDefault(D2DContext& cxt, INT_PTR wp, float xoff )
{
	D2DMatrix mat(cxt);	
		
	mat_ = mat.PushTransform();
	//mat.Scale(scale_,scale_);
			
	FRectF rcborder = rc_.GetBorderRect();
			
	//cxt.cxt->DrawRectangle( rcborder, br );
	cxt.cxt->FillRectangle( rcborder, DRGB(back_) );
			
	FRectF rc = rc_.GetContentRect();
			
	D2DRectFilter f(cxt, rc); 
			
	mat.Offset( rcborder.left + xoff, rcborder.top );					
						
	{				
		DefPaintWndProc(parent_,WM_PAINT,wp,nullptr);
	}

	mat.PopTransform();

}

int D2DSlideControls::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	//if ( IsHide())
	//	return 0;
	
	int ret = 0;
		
	switch( message )
	{
		case WM_PAINT:
		{
			D2DContext& cxt = *(d->cxt());

			float xoff = 0;			
			
			if ( bSliding_ || cc_ !=nullptr )
			{
				float w = rc_.Width();
				FRectF rcs(0,0,w,0);
				FRectF rcz(w,0,w*2, 0);
				d->redraw();
				
				FRectF rc;
				cc_ = sybil::DrawDriftRect( cc_, &rc, cxt.tickcount_, rcs, rcz, 500 );
				if ( cc_ == nullptr )
				{
					// Drift complete.

					Sliding(true);
				}
				else
				{
					xoff = (md_ == 1 ? -rc.left : rc.left-rc.Width() );										
				}
			}
			
			OnDrawDefault( cxt, wp, xoff );
					
			return 0;
		}
		break;

		case WM_LBUTTONDOWN:
		{
			FPointF pt3 = mat_.DPtoLP(lp);
			if ( rc_.PtInRect(pt3 ))
			{
				
				if ( pt3.y < 10 )
				{
					Sliding(false);

					ret = 1;
					d->redraw();
				}
				else
					ret = D2DControls::DefWndProc(d,message,wp,lp);
			}

		}
		break;
		case WM_MOUSEMOVE:
		ret = D2DControls::DefWndProc(d,message,wp,lp);
		break;
		case WM_LBUTTONUP:
		ret = D2DControls::DefWndProc(d,message,wp,lp);
		break;



		case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		{
			FRectF rc = parent_control_->GetRect();
			rc_.SetSize( rc.Size());

			
			D2DControls::DefWndProc(d,message,wp,lp);
		}
		break;

		default :
			ret = D2DControls::DefWndProc(d,message,wp,lp);
	}

	return ret;

}

void D2DSlideControls::Sliding( bool finish )
{
	if ( finish )
	{
		// どっちかを消して、(0,0)に配置

		if ( md_ == 1 )
		{
			left_->Hide();
			right_->Visible();
		}
		else
		{
			left_->Visible();
			right_->Hide();
		}

		auto rc = left_->GetRect();
		right_->SetRect( rc );
		bSliding_ = false;
	}
	else
	{
		md_ = ( right_->IsHide() ? 1 : 2 );

		// 横に並べる
		bSliding_ = true;
		left_->Visible();
		right_->Visible();

		auto rc = left_->GetRect();
		rc.Offset( rc.Width(), 0 );
		right_->SetRect( rc );
	
	}
}
