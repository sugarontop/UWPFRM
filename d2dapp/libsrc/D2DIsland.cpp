#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DIsland.h"

using namespace V4;

void SoftSqueeze(D2DControl* p, FRectF* prc, FRectF frc, FRectF trc, int milisec);


void DrawText::Text(D2DContext& cxt, LPCWSTR str)
{
	t_.Release();
	cxt.wfactory->CreateTextLayout(str, wcslen(str), cxt.textformat, 1000, 1000, &t_);
}
void DrawText::d(D2DContext& cxt, const FPointF& pt, ID2D1Brush* br)
{
	cxt.cxt->DrawTextLayout(pt,t_,br);
}




D2DIsland::D2DIsland()
{
	

}
void D2DIsland::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	mode_ = MODE::NORMAL;

	InnerCreateWindow(pacontrol, rc, stat, name, local_id);
	

	auto cxt = GetParentWindow()->cxt();
	title_.Text(*cxt, name);
}


int D2DIsland::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if (mode_ == MODE::NORMAL)
		return WndProcN(d, message, wp, lp);
	else
		return WndProcB(d, message, wp, lp);
}





int D2DIsland::WndProcN(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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

			FRectF rc(rc_);
			cxt.cxt->DrawRectangle(rc, cxt.blue);

			

			mat.Offset( rc_.left, rc_.top );

			FRectF rct(20,rc_.Size().height-30,FSizeF(rc_.Size().width, 30));
			//cxt.cxt->DrawText(name_.c_str(),name_.length(),cxt.textformat,rct,cxt.black);
			title_.d(cxt,rct.LeftTop(), cxt.black);


			DefPaintWndProc(d, message, wp, lp);

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_LBUTTONUP:
		{
			LOGPT(pt3,wp)

			if (!rc_.PtInRect(pt3) && this == GetCapture())
			{
				ReleaseCapture(this);
				ret = 1;
			}
		}
		break;

		case WM_D2D_NCHITTEST:
		{			
			LOGPT(pt3,wp)

			if (rc_.PtInRect(pt3))
			{
				ret = HTCLIENT;
			}
		}
		break;
		case WM_D2D_MOUSEACTIVATE:
		{
			LOGPT(pt3,wp)

			if (rc_.PtInRect(pt3))
			{
				ret = MA_ACTIVATE;
			}
		}
		break;
		
	}

	if ( ret == 0 )
		ret = D2DControls::DefWndProc(d,message,wp,lp);

	return ret;

}
void D2DIsland::ModeChange()
{
	mode_ = (mode_ == MODE::NORMAL ? MODE::BIT : MODE::NORMAL);
}
int D2DIsland::WndProcB(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	int ret = 0;

	if (IsHide() && !IsImportantMsg(message))
		return 0;

	switch (message)
	{
		case WM_PAINT:
		{
			auto& cxt = *(d->cxt());
			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();

		
			

			ComPTR<ID2D1SolidColorBrush> br;
			cxt.cxt->CreateSolidColorBrush(D2RGB(220, 220, 220), &br);
			
			cxt.cxt->FillRectangle(rcMin_, br );

			mat.Offset(rcMin_.left, rcMin_.top);

			FRectF rct(10, 0, FSizeF(rcMin_.Size().width, 30));			
			title_.d(cxt, rct.LeftTop(), cxt.black);
			
			mat.PopTransform();
			
			return 0;
		}
		break;
		case WM_LBUTTONDBLCLK:
		{
			LOGPT(pt3,wp);

			if (rcMin_.PtInRect(pt3))
			{
				mode_ = (mode_ == MODE::NORMAL ? MODE::BIT : MODE::NORMAL);
				ret = 1;

				
			}
			
		}
		break;
	}

	//if (ret == 0)
	//	ret = D2DControls::DefWndProc(d, message, wp, lp);

	return ret;
}



class SqueezeOne
{
	public :
		SqueezeOne(){}

		static FRectF* liner_timeframe(FRectF frc, FRectF trc, int stepcount )
		{
			FRectF* r = new FRectF[stepcount];

			for(int i = 0; i < stepcount; i++ )
			{
				FRectF& rk = r[i];

				rk.left = frc.left + i*(trc.left - frc.left) / stepcount;
				rk.top = frc.top + i * (trc.top - frc.top) / stepcount;
				rk.right = frc.right + i * (trc.right - frc.right) / stepcount;
				rk.bottom = frc.bottom + i * (trc.bottom - frc.bottom) / stepcount;
			}
			r[stepcount -1] = trc;
			
			return r;
		}


		static DWORD CALLBACK anime(LPVOID p)
		{
			SqueezeOne* ps = (SqueezeOne*)p;

			for (int i = 0; i < ps->stepcount_; i++)
			{
				int ii = ps->step_++;
				for( auto& it : ps->sqrects_ )
				{
					FRectF rc = (*it).prc_[ii];

					*((*it).target) = rc;
				}

				ps->win_->InvalidateRect();
				Sleep(16);
			}

			delete ps; // 
			return 0;
		}

		void Start(D2DWindow* pw, const std::vector<std::shared_ptr<RectSqueeze>>& rc, int stepcount)
		{
			for(auto& it : rc )
			{
				(*it).prc_ = liner_timeframe((*it).frc, (*it).trc, stepcount);
				*(*it).target = (*it).frc;
			}

			step_ = 0;
			stepcount_ = stepcount;
			win_ = pw;
			sqrects_ = rc;



			DWORD dw;
			CreateThread(0, 0, anime, this, 0, &dw);
		}
	protected :		
		int step_, stepcount_;
		D2DWindow* win_;
		FRectF* target_;

		std::vector<std::shared_ptr<RectSqueeze>> sqrects_;

};

void SoftSqueeze(D2DWindow* p, const std::vector<std::shared_ptr<RectSqueeze>>& ar, int milisec)
{
	// 60fps
	int cnt = (int)(milisec / 60.0f);

	SqueezeOne* so = new SqueezeOne();
	so->Start(p, ar, cnt);


}




