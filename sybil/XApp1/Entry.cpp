#include "pch.h"
#include "D2DUniversalControlBase.h"
#include "Entry.h"

using namespace V4;
using namespace V4_XAPP1;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D2DButton::DefaultDrawButton( D2DButton* sender, D2DContext& cxt )
{
	FRectF rc = sender->rc_.GetBorderRectZero();


	LPCWSTR str =L"V4_XAPP1::D2DButton";
	int strlen = wcslen(str); 
	
	ID2D1LinearGradientBrush* br = cxt.silver_grd;
	
	br->SetStartPoint( FPointF(0,rc.top));
	br->SetEndPoint( FPointF(0,rc.bottom));

	ComPTR<ID2D1SolidColorBrush> br2;
	cxt.cxt->CreateSolidColorBrush(D2RGBA(230,230,230,200 ), &br2 );
	
	//FillRectangle( cxt, rc, br2 );
	cxt.cxt->FillRectangle( rc, br2 );

	FRectF rc2(rc);
	rc2.InflateRect( -2,-2);

	//FillRectangle( cxt, rc2, br );
	cxt.cxt->FillRectangle( rc2, br );

	//CenterTextOut( cxt.cxt.p, rc, str, strlen, cxt.textformats[0], cxt.black );

	auto tf = cxt.textformats[0];
	auto old = tf->GetTextAlignment();
	tf->SetTextAlignment( DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER );	
	cxt.cxt->DrawText( str, strlen, tf, rc, cxt.black, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
	tf->SetTextAlignment( old );

}

void D2DButton::Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR title, LPCWSTR name, int controlid)
{
	InnerCreateWindow(parent,pacontrol,rc,stat,name, controlid);
	mode_ = 0;
	title_ = title;
	OnPaint_ = DefaultDrawButton;
}

int D2DButton::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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
			mat.Offset(rc_.left, rc_.top);


			OnPaint_( this, cxt );


			mat.PopTransform();
		}
		break; 
		case WM_LBUTTONDOWN:
		{
			Windows::UI::Core::PointerEventArgs^ arg = (Windows::UI::Core::PointerEventArgs^)lp;
			
			FPointF pt(arg->CurrentPoint->Position);
			
			FPointF pt3 = mat_.DPtoLP(pt);

			mode_ = 0;

			if ( rc_.PtInRect(pt3 ))
			{
				mode_ = 1;
				parent_control_->SetCapture( this );
				ret = 1;

				auto x = GetParentWindow();				
			}

			
		}
		break;
		case WM_KEYDOWN :
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;

			if ( arg->VirtualKey == Windows::System::VirtualKey::Escape && parent_control_->GetCapture() == this )
			{
				parent_control_->ReleaseCapture();
				mode_ = 0;				
			}
		}
		break;
		case WM_LBUTTONUP :
		{
			if ( parent_control_->GetCapture() == this )
			{
				parent_control_->ReleaseCapture(this, -1);
				mode_ = 0;
				ret = 1;

				Windows::UI::Core::PointerEventArgs^ arg = (Windows::UI::Core::PointerEventArgs^)lp;

				FPointF pt;
				pt.x = arg->CurrentPoint->Position.X;
				pt.y = arg->CurrentPoint->Position.Y;
			
				FPointF pt3 = mat_.DPtoLP(pt);
							
				if ( rc_.PtInRect(pt3 ))
				{
					if ( OnClick_ )
						OnClick_( this );
				}
			}
		}
		break;

	}
	return ret;
}