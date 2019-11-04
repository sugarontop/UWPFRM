#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
//#include "D2DDriftDialog.h"

using namespace V4;

#define BARWIDTH 18

#define SCBAR(x) ((D2DScrollbar*)x.get())


#pragma region  D2DScrollbar
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D2DScrollbar::D2DScrollbar()
{
	info_.button_height = 16;
	info_.stat = D2DScrollbarInfo::STAT::NONE;
	info_.position = 0;
	info_.total_height = 0; 
	info_.bVertical = true;
	info_.auto_resize = true;
	target_control_ = nullptr;
	info_.clr[0] = nullptr;
	info_.clr[1] = nullptr;
	info_.clr[2] = nullptr;
	

}
void D2DScrollbar::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol,rc,stat,name, local_id);

	target_control_ = pacontrol;
	OnCreate();
}
void D2DScrollbar::CreateWindowEx(D2DWindow* parent, D2DControl* target, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	parent_ = parent;
	target_control_ = target;
	rc_ = rc;
	stat_ = stat;
	name_ = name;
	id_ = id;

	parent_control_ = nullptr;


	OnCreate();

}
void D2DScrollbar::SetRowHeight( float rowheight )
{
	info_.row_height = rowheight; 
}


void D2DScrollbar::SetScrollBarColor( D2DContext& cxt )
{
	ID2D1SolidColorBrush *bkcolor,*br1b,*br2;
	cxt.cxt->CreateSolidColorBrush(D2RGBA(230,230,230,255 ), &bkcolor );
	cxt.cxt->CreateSolidColorBrush(D2RGBA(200,200,200,255 ), &br1b );
	cxt.cxt->CreateSolidColorBrush(D2RGBA(100,100,100,255 ), &br2 );	
		
	info_.clr[0] = bkcolor;
	info_.clr[1] = br1b;
	info_.clr[2] = br2;
}

void D2DScrollbar::OnDXDeviceLost() 
{ 
	for(int i = 0; i < 3; i++ ) 
		info_.clr[i]->Release();
}
void D2DScrollbar::OnDXDeviceRestored(D2DContext& cxt)
{ 
	SetScrollBarColor( cxt);
}
void D2DScrollbar::SetScrollbarTop()
{
	info_.position = 0;
	if (target_control_)
		target_control_->UpdateScrollbar( this );			

}

void D2DScrollbar::OnCreate()
{
	info_.row_height = 1.0f;
	info_.rowno = 0;
	info_.bVertical = (rc_.Width() < rc_.Height());		
	info_.total_height = ( info_.bVertical ? rc_.Height() : rc_.Width()); 

	SetScrollBarColor( *parent_->cxt());

	OtherHand(false);
}

void D2DScrollbar::OtherHand(bool bl)
{ 
	OtherHand_ = bl; 

	FRectF rc = rc_.GetContentRect();
	if ( OtherHand_ ) 
	{
		if ( info_.bVertical )
			rc.bottom -= BARWIDTH;
		else
			rc.right -= BARWIDTH;
	}

	info_.rc = rc;
}

const D2D1_RECT_F& D2DScrollbar::GetContentRect()
{
	return info_.rc;
}




int D2DScrollbar::WndProc(D2DWindow* d, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam)
{
	if ( IsHide() ) 
		return 0;

	int ret = 0;
	
	switch( message )
	{
		case WM_PAINT:
		{
			if ( wParam == 0 )
			{
				D2DContext& cxt = *(d->cxt());

				D2DMatrix mat(cxt);	
		
				mat.PushTransform();
						
				info_.rc = rc_.GetContentRect();
									
				cxt.cxt->GetTransform( &mat_ );
				DrawScrollbar( cxt, info_ );

				mat.PopTransform();
			}
			return 0;
		}
		break;
		case WM_D2D_NCHITTEST:
		{
			FPointF& pt = *(FPointF*)(wParam);
			FPointF pt3 = mat_.DPtoLP(pt);
			if (rc_.PtInRect(pt3))
			{
				ret = HTCLIENT;
			}
		}
		break;
		case WM_D2D_MOUSEACTIVATE:
		{
			FPointF& pt = *(FPointF*)(wParam);
			FPointF pt3 = mat_.DPtoLP(pt);
			if (rc_.PtInRect(pt3))
			{
				ret = MA_ACTIVATE;
			}			
		}
		break;
			
		case WM_LBUTTONDOWN:
		{
			auto& cxt = *(d->cxt());

			FPointF pt2 = mat_.DPtoLP( FPointF(lParam)); // 親座標で変換
			info_.ptprv = pt2;
			int stat = D2DScrollbarInfo::STAT::NONE;
			if ( rc_.PtInRect(pt2))
			{
				
				parent_control_->SetCapture(this);

				FRectF rc1 = ScrollbarRect( info_,1 );
				FRectF rc2 = ScrollbarRect( info_,2 );
				FRectF rc = ScrollbarRect( info_,3 );
				if ( rc.PtInRect(pt2 ) ) 
					stat = D2DScrollbarInfo::STAT::ONTHUMB;
				else if ( rc1.PtInRect(pt2 ) ) 
					stat = D2DScrollbarInfo::STAT::ONBTN1;				
				else if ( rc2.PtInRect(pt2 ) ) 
					stat = D2DScrollbarInfo::STAT::ONBTN2;
				else
				{
					float thumb_height_c = info_.thumb_rc.bottom - info_.thumb_rc.top;
					
					FRectF rcc1( rc1.left,rc1.bottom, rc1.right,  rc1.bottom + info_.position );
					FRectF rcc2( rc1.left,rc1.bottom+info_.position+thumb_height_c, rc1.right, rc2.top );
					if ( rcc1.PtInRect(pt2 ) ) 
						stat = D2DScrollbarInfo::STAT::ONSPC1;	
					else if ( rcc2.PtInRect(pt2 ) ) 
						stat = D2DScrollbarInfo::STAT::ONSPC2;	
				}
				stat |= D2DScrollbarInfo::STAT::CAPTURED;

				ret = 1;
			}

			if ( info_.stat != stat )
			{
				info_.stat = stat;
			}
			
		}
		break;
		case WM_MOUSEMOVE:
		{
			FPointF pt2 = mat_.DPtoLP( FPointF(lParam)); 
			
			int stat = D2DScrollbarInfo::STAT::NONE;
			if ( IsCaptured())
			{
				stat = D2DScrollbarInfo::STAT::ONTHUMB;

				FRectF rc1 = ScrollbarRect( info_,1 );
				FRectF rc2 = ScrollbarRect( info_,2 );
				FRectF rc = ScrollbarRect( info_,3 );
				//if ( rc.PtInRect(pt2 ) ) 
				//	stat = D2DScrollbarInfo::STAT::ONTHUMB;
				if ( rc1.PtInRect(pt2 ) ) 
					stat = D2DScrollbarInfo::STAT::ONBTN1;
				else if ( rc2.PtInRect(pt2 ) ) 
					stat = D2DScrollbarInfo::STAT::ONBTN2;		
			}
			
			float off = 0;
			// キャプチャーしていたらスクロールは有効とみなす
			if ( IsCaptured() && stat & D2DScrollbarInfo::STAT::ONTHUMB )
			{
				FPointF prev = info_.ptprv;// d->CapturePoint( pt2 );
					
				if ( info_.bVertical )
				{					
					off = pt2.y-prev.y;

					if ( off )
					{
						FRectF rc = GetContentRect();
						float move_area = rc.Height()-info_.button_height*2;
						float thumb_height_c = info_.thumb_rc.bottom - info_.thumb_rc.top;

						if ( info_.position + off + thumb_height_c <= move_area )
						{
							info_.position = max(0,info_.position + off );									
						}						
					}
				}
				else
				{
					off = pt2.x-prev.x;
					if ( off )
					{
						float move_area = rc_.Width()-info_.button_height*2;
						float thumb_height_c = info_.thumb_rc.right - info_.thumb_rc.left;

						if ( info_.position + off + thumb_height_c < move_area )
							info_.position = max(0,info_.position + off );							
					}
				}
				
				if ( off )
				{
					if (target_control_)
						target_control_->UpdateScrollbar( this );										

					d->redraw();
				}
				
				stat |= D2DScrollbarInfo::STAT::CAPTURED;
				ret = 1;
			}
			else
			{
				stat &= ~D2DScrollbarInfo::STAT::CAPTURED;
			}

			if (info_.stat != stat)
			{
				info_.stat = stat;
				d->redraw();
			}
			info_.ptprv = pt2;
		}
		break;		
		case WM_CAPTURECHANGED:
		case WM_LBUTTONUP:
		{
			if ( IsCaptured())
			{
				parent_control_->ReleaseCapture();

				info_.stat &= ~D2DScrollbarInfo::STAT::CAPTURED;

				float off = 0;
				if ( info_.stat == D2DScrollbarInfo::STAT::ONBTN1 )
					off = -OffsetOnBtn(1);
				else if ( info_.stat == D2DScrollbarInfo::STAT::ONBTN2 )
					off = OffsetOnBtn(1);
				else if ( info_.stat == D2DScrollbarInfo::STAT::ONSPC1 )
					off = -OffsetOnBtn(2);
				else if ( info_.stat == D2DScrollbarInfo::STAT::ONSPC2 )
					off = OffsetOnBtn(2);

				if ( off )
				{
					float move_area = 0, thumb_height_c = 0;
					if ( info_.bVertical )
					{
						FRectF rc = GetContentRect();
						move_area = rc.Height()-info_.button_height*2;
						thumb_height_c = info_.thumb_rc.bottom - info_.thumb_rc.top;
					}
					else
					{
						move_area = rc_.Width()-info_.button_height*2;
						thumb_height_c = info_.thumb_rc.right - info_.thumb_rc.left;
					}
						
					info_.position = min( move_area -thumb_height_c, max(0,info_.position + off ));
					if (target_control_)
						target_control_->UpdateScrollbar( this );										
				}
				ret = 1;
			}
			else
			{
				info_.stat &= ~D2DScrollbarInfo::STAT::CAPTURED;
				info_.stat = D2DScrollbarInfo::STAT::NONE;
			}

			
		}
		break;
		case WM_MOUSEWHEEL:
		{
			Windows::UI::Core::PointerEventArgs^ arg = (Windows::UI::Core::PointerEventArgs^)lParam;

			{
				float delta = arg->CurrentPoint->Properties->MouseWheelDelta / -120.0f * 2;

				if ( info_.bVertical )
				{
					float npos = info_.position + delta;

														
					
					info_.position = max(0,npos);
					info_.rowno += ( npos > 0 ? 1 : -1 );
					
								
					

					ret = 1;

					if (target_control_)
						target_control_->UpdateScrollbar( this );	

					d->redraw();
				}
				
			}


		}
		break;
		case WM_DESTROY:
		{
			OnDXDeviceLost();

		}
		break;
		case WM_SIZE:
		{
			//ScrollbarRect( info_, 3 );
		}
		break;
		
	}	
	return ret;
}

float D2DScrollbar::OffsetOnBtn( int typ )
{	
	if ( typ == 1  )
		return  10; 
	else if ( typ == 2 )
		return 20; 

	return 0;
}
void D2DScrollbar::SetTotalSize( float cy )
{
	_ASSERT( 0.0f <= cy  );
	info_.total_height = cy;
}

#pragma endregion