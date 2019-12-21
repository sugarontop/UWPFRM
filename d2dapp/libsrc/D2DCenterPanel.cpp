#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DTextbox.h"
#include "D2DCenterPanel.h"

using namespace V4;
using namespace concurrency;


D2DCenterPanel::D2DCenterPanel()
{
	a_ = 0;
}
int D2DCenterPanel::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	int ret = 0;

	if (IsHide() && !IsImportantMsg(message))
		return 0;

	switch (message)
	{
		case WM_PAINT:
		{
			CXTM(d)

			mat_ = mat.PushTransform();
			
			D2DRectFilter fil(cxt, rcOrginal_);

			mat.Offset(rc_.left + a_, rc_.top);

			{
				cxt.cxt->FillRectangle(rc_.ZeroRect(), cxt.ltgray2);

				cxt.cxt->DrawTextLayout(FPointF(0,0), text_, cxt.ltgray);

				DefPaintWndProc(d, message, wp, lp);
			}

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		{
			FSizeF sz = *(FSizeF*)wp;
			auto pt = rc_.CenterPt();
			rc_.Offset( -pt.x + sz.width/2, -pt.y+sz.height/2); 
			rcOrginal_ = rc_;
		}
		break;
		case WM_D2D_MOUSEACTIVATE:
		{
			LOGPT(pt3, wp);
			if (rc_.PtInRect(pt3))
			{
				ret = MA_ACTIVATE;
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			LOGPT(pt3, wp);
			if (rc_.PtInRect(pt3))
			{
						
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			LOGPT(pt3, wp);
			if (rc_.PtInRect(pt3))
			{
				if ( a_ == 0 )
				{
					auto off = 50 - rc_.Size().width;
					
					V4::MoveMatrixY(GetParentWindow(), &a_, off);

				}
				else if (a_ )
				{
					auto off = 50 - rc_.Size().width;

					V4::MoveMatrixY(GetParentWindow(), &a_, -off);


				}

				ret = 1;
			}
		}
		break;

	}

	if (ret == 0)
		ret = D2DControls::DefWndProc(d, message, wp, lp);

	return ret;

}
void D2DCenterPanel::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol, rc, stat, name, local_id);

	CXTM(pacontrol->GetParentWindow())

	LPCWSTR str = L"5代目は、北米市場において2018年11月に発表、ガソリン車は同年12月より販売を開始したほか、ハイブリッ\nド車は2019年3月下旬より販売開始となることが発表となった";

	CreateTextLayout(cxt, str, wcslen(str), &text_);


}