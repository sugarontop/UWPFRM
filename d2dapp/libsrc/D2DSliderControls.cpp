#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DSliderControls.h"
#include "D2DIsland.h"


using namespace V4;

#define LEFTBTN_WIDTH 10




D2DSliderControls::D2DSliderControls()
{

}
int D2DSliderControls::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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
					
			D2DRectFilter fl(cxt,rc);

			cxt.cxt->FillRectangle(rc, cxt.black);

			rc.left=0; rc.right = LEFTBTN_WIDTH;
			cxt.cxt->FillRectangle(rc, cxt.ltgray);

			mat.Offset(rc_.left, rc_.top);

			
			FRectF rcimg(40, 0, bmp_->GetSize());
			cxt.cxt->DrawBitmap(bmp_, rcimg);


			DefPaintWndProc(d, message, wp, lp);

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		{
			FSizeF& sz = *(FSizeF*)wp;

			const float h = Height_;

			rcMax_.top = sz.height-h;
			rcMax_.bottom = sz.height;
			rcMax_.left=0;
			rcMax_.right = sz.width;
			
			if (bOpen_)
				rc_ = rcMax_;


			DefPaintWndProc(d, message, wp, lp);
		
			if ( message == WM_D2D_INIT_UPDATE )
			{
				Image img;
				auto cxt = d->cxt()->cxt;
				img.LoadImage(_u("assets/test.png"));
				img.GetImage(cxt, &bmp_);
			}



			return 0;
		}
		break;
		case WM_LBUTTONUP:
		{
			LOGPT(pt3, wp)

			if (rc_.PtInRect(pt3))
			{
				if ( pt3.x < LEFTBTN_WIDTH )
				{
					
										
					//rc_.SetWidth( bOpen_ ? rcMax_.Width() :  LEFTBTN_WIDTH);

					DrawSqueeze();
					bOpen_ = !bOpen_;

					ret = 1;
				}
				
			}

		}
		break;
		case WM_D2D_THREAD_COMPLETE:
		{
			if ((INT_PTR)this == wp)
			{
				//if (isModal_)
				{
					//if (IsHide())
					{
						GetParentControl()->ReleaseCapture();
					}
					
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
void D2DSliderControls::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol, rc, stat, name, local_id);

	rcMax_ = rc;
	Height_ = rc.Height();
	bOpen_ = true;

}
static DWORD CALLBACK _anime(LPVOID p)
{
	FRectF3* sb = (FRectF3*)p;
	D2DSliderControls* psb = (D2DSliderControls*)sb->obj;

	const int step_cont = 70;

	float step = (sb->trc.left - sb->frc.left) / step_cont;


	FRectF rc1 = sb->frc;
	for (int i = 0; i < step_cont; i++)
	{
		psb->SetRect(rc1);

		//if (i == 0)
		//	psb->Visible();

		rc1.Offset(step, 0);

		psb->GetParentWindow()->InvalidateRect();
		Sleep(16);
	}


	psb->SetRect(sb->trc);

	//if (sb->trc.left < 0)
	//	psb->Hide();

	delete sb;

	psb->GetParentWindow()->PostMessage(WM_D2D_THREAD_COMPLETE, (INT_PTR)psb, nullptr);

	return 0;
};
void D2DSliderControls::DrawSqueeze()
{
	
	GetParentControl()->SetCapture(this);
	
	bool isShow = bOpen_; // (stat_ & STAT::VISIBLE);
	FRectF3* pf = new FRectF3();

	if (!isShow)
	{
		FRectF3 f;
		f.lastrc = rcMax_;
		f.trc = rcMax_;

		f.frc = rcMax_;
		f.frc.Offset(-rcMax_.Width(), 0);
		f.obj = this;

		*pf = f;


		SetRect(f.frc);
		//Visible();
	}
	else
	{
		FRectF3 f;

		f.trc = rcMax_;
		f.trc.Offset(-rcMax_.Width()+ LEFTBTN_WIDTH, 0);

		f.lastrc = f.trc;
		f.frc = rc_;
		f.obj = this;

		*pf = f;
	}

	DWORD dw;
	CreateLightThread(_anime, pf, &dw);
}