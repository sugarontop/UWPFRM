#include "pch.h"
#include "D2DDevelop.h"
#include "D2DCommon.h"

using namespace V4;



void D2DVerticalbarControls::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, D2D1_COLOR_F* clr, int clrcnt )
{
	D2DWindow* win = pacontrol->GetParentWindow();
	InnerCreateWindow(win,pacontrol,rc,stat, name, -1);

	_ASSERT(clrcnt==3);
	for( int i = 0; i < clrcnt; i++ )
		clr_[i] = clr[i];

}
int D2DVerticalbarControls::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( !IsVisible())
		return 0;

	int ret = 0;
	
	switch( message )
	{
		case WM_PAINT :
		{
			auto& cxt = *(d->cxt());
			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();
			mat.Offset(rc_.left, rc_.top);

			OnPaint( cxt );

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_INIT_UPDATE:
		case WM_SIZE :
		{
			if ( wmsize_ )
			{
				rc_ = wmsize_(this);
			}
			else
			{
				auto rc = GetParentControl()->GetRect();
				rc_.SetSize(rc.GetSize());
			}

			DefPaintWndProc(d,message,wp,lp);
			return 0;
		}
		break;
	}

	if ( ret == 0 )
		ret = DefWndProc(d,message,wp,lp);
	return ret;

}



void D2DVerticalbarControls::OnPaint(D2DContext& cxt)
{
	auto rc = rc_.ZeroRect();

	auto br1 = CreateBrush(cxt, clr_[0]);
	auto br3 = CreateBrush(cxt, clr_[2]);

	cxt.cxt->FillRectangle(rc, br1 );

	rc.InflateRect(-3,-3);
	cxt.cxt->FillRectangle(rc, br3 );

	
	rc.InflateRect(-1,-1);

	OnTestButton(cxt, rc);

}
void D2DVerticalbarControls::OnTestButton(D2DContext& cxt, FRectF& rc)
{
	auto br11 = CreateBrush(cxt, clr_[0]);

	cxt.cxt->FillRectangle(rc, br11 );

	D2D1_ROUNDED_RECT rrc;
	FRectF btnrc(25,100,FSizeF(50,25));
	rrc.rect = btnrc;
	rrc.radiusX = 10;
	rrc.radiusY = 10;

	ComPTR<IDWriteTextLayout> tl;
	FSizeF sz = CreateTextLayout( cxt, L"b1", 2, &tl );
	

	for( int i = 0; i < 3; i++ )
	{
		auto ptc = btnrc.TextLayoutPt(sz);

		cxt.cxt->DrawRoundedRectangle( rrc, cxt.black );
		cxt.cxt->FillRoundedRectangle( rrc, cxt.bluegray  );
		
		cxt.cxt->DrawTextLayout( ptc, tl, cxt.white );


		btnrc.Offset(0,50);
		rrc.rect = btnrc;

	}
}