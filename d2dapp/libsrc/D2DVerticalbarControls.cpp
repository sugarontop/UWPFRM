#include "pch.h"
#include "D2DDevelop.h"
#include "D2DCommon.h"

using namespace V4;



void D2DVerticalbarControls::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, D2D1_COLOR_F* clr, int clrcnt )
{
	
	InnerCreateWindow(pacontrol,rc,stat, name, -1);

	_ASSERT(clrcnt==3);
	for( int i = 0; i < clrcnt; i++ )
		clr_[i] = clr[i];

}
int D2DVerticalbarControls::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( IsHide() && !IsImportantMsg(message) )
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

			DefPaintWndProc(d,message,wp,lp);

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

	cxt.cxt->FillRectangle(rc, br1 );

	//OnTestButton(cxt, rc, br1);


}
