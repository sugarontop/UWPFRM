#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DTextbox.h"
#include "D2DFooterbar.h"

using namespace V4;
using namespace concurrency;


D2DFooterbar::D2DFooterbar()
{

}
int D2DFooterbar::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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
			D2DRectFilter fil(cxt, rc_);


			auto blue = V4::CreateBrush(cxt, D2RGB(0,122,204));
			cxt.cxt->FillRectangle(rc_, blue);

			mat.Offset(rc_.left, rc_.top);
			{

				DefPaintWndProc(d, message, wp, lp);
			}

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		{
			FSizeF* psz = (FSizeF*)wp;
						
			auto szWindow = GetParentControl()->GetRect().Size(); 

			rc_.SetWidth(szWindow.width);

			rc_.top = szWindow.height - rc_.Height();
			rc_.bottom = szWindow.height;

			float offx = 0;
			auto cx = szWindow.width / controls_.size();
			for(auto& it : controls_)
			{
				FRectF rc(0,0,cx, rc_.Height());

				rc.Offset(offx, 0);
				it->SetRect(rc);

				offx += cx;
			}

			
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
	

	}

	if (ret == 0)
		ret = D2DControls::DefWndProc(d, message, wp, lp);

	return ret;

}
void D2DFooterbar::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol, rc, stat, name, local_id);
}







