#include "pch.h"
#include "D2DWindowMessage.h"
#include "D2DUniversalControl.h"
#include "D2DCommon.h"
using namespace V4;


void D2DChildFrame::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol, rc, stat, name, local_id);


}

int D2DChildFrame::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if (IsHide() && !IsImportantMsg(message))
		return 0;

	int ret = 0;


	switch (message)
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());
			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();

			auto rc = this->GetRect();
			cxt.cxt->FillRectangle(rc,cxt.bluegray);

			FRectF rc1(rc.right, rc.top, rc.right, rc.top);
			rc1.left -= 10;
			rc1.bottom += 10;
			cxt.cxt->FillRectangle(rc1, cxt.red);
			

			mat.Offset(rc.left, rc.top);


			DefPaintWndProc(d, WM_PAINT, wp, nullptr);

			mat.PopTransform();
			return 0;

		}
		break;
		case WM_D2D_NCHITTEST:
		{
			FPointF& pt = *(FPointF*)(wp);
			FPointF pt3 = mat_.DPtoLP(pt);
			if (rc_.PtInRect(pt3))
			{
				ret = HTCLIENT;
			}
		}
		break;
		case WM_D2D_MOUSEACTIVATE:
		{
			FPointF& pt = *(FPointF*)(wp);
			FPointF pt3 = mat_.DPtoLP(pt);
			if (rc_.PtInRect(pt3))
			{
				ret = DefWndProc(d, message, wp, lp);

				if ( ret == 0 )
					ret = MA_ACTIVATE;
				return ret;
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(lp);
			if (rc_.PtInRect(pt))
			{
				FRectF rc1(rc_.right, rc_.top, rc_.right, rc_.top);
				rc1.left -= 10;
				rc1.bottom += 10;

				if ( rc1.PtInRect(pt))
				{
					GetParentControl()->SetCapture(this);
					ret = 1;
				}
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			FPointF pt(lp);
			FPointF pt3 = mat_.DPtoLP(lp);

			if ( this == GetParentControl()->GetCapture())
			{
				WParameterMouse* prvm = (WParameterMouse*)wp;

				rc_.Offset(pt.x - prvm->move_ptprv.x, pt.y - prvm->move_ptprv.y);

				ret = 1;
				d->redraw();
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			if (this == GetParentControl()->GetCapture())
			{
				GetParentControl()->ReleaseCapture();
				ret = 1;
			}
		}
		break;

	}

	if (ret == 0)
		ret = D2DControls::DefWndProc(d, message, wp, lp);
	
	return ret;
}