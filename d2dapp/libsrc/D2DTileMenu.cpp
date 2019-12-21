#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DIsland.h"
#include "D2DTileMenu.h"

using namespace V4;
using namespace concurrency;


D2DTileMenu::D2DTileMenu()
{

}
int D2DTileMenu::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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

			//cxt.cxt->FillRectangle(rc_, cxt.gray);
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
			auto szWindow = *(FSizeF*)wp;
			//rc_.SetSize(800,300); //szWindow.width, szWindow.height-200);

			FRectF xrc(FPointF(), szWindow);

			rc_.MoveCenter(xrc);


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
				int a = 0;
			}
		}
		break;
		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;

			switch (arg->VirtualKey)
			{
				case Windows::System::VirtualKey::Escape:
				{
					if (parent_control_->GetCapture() == this )
						parent_control_->ReleaseCapture();
				}
			break;
			}


		}
		break;
		

	}

	if (ret == 0)
		ret = D2DControls::DefWndProc(d, message, wp, lp);

	return ret;

}
void D2DTileMenu::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol, rc, stat, name, local_id);

	test_ = 0;
}


#include "D2DTab.h"
void D2DTileMenu::Test(int pagecnt)
{
	int k = 1;

	WCHAR xw[] = {L'A',L'B',L'C',L'D'}; 

	for(int i = 0; i < pagecnt; i++ )
	{
		FRectF rc(200,200,FSizeF(800,300));
		D2DTabContent* island = new D2DTabContent();
		island->Create(this, NONE, NONAME, 1 );

		xar_.push_back(island); // controls_ÇÕcaptureÇ≈èáî‘ÇïœÇ¶ÇÈÇÃÇ≈Ç‚Ç‚Ç±ÇµÇ¢ÇÃÇ≈égÇÌÇ»Ç¢

		FRectF rcb(50,50,FSizeF(100,30));
		for( int r =0; r < 4; r++ )
		{			
			FRectF rcb1 = rcb;
			
			for(int c=0; c< 4; c++ )
			{
				auto nm = V4::Format(L"%c-%d%d", xw[i], r, c );

				D2DButton* btn = new D2DButton();
				btn->Create(island, rcb1, VISIBLE, nm.c_str(), nm.c_str(), k++ );

				btn->OnClick_ = [this, island](D2DButton* btn){					
					
					auto nm = btn->GetName();

					
					FRectF rc;//(100,100,FSizeF(300,200));
					V4::D2DMessageBox::Show(this, rc, L"hoi", nm.c_str() );
					

					/*FRectF rc = btn->GetRect();

					xar_[0]->Hide();
					xar_[1]->Hide();
					xar_[2]->Hide();
					xar_[3]->Hide();

					{
						test_ = min(test_ + 1, 3);
						xar_[test_]->Visible();
					}

					auto mat = island->GetMat();
					rc = mat.LPtoDP(rc);

					auto mat2 = GetParentControl()->GetMat();
					rc = mat2.DPtoLP(rc);	   

					D2DButton* nbtn = new D2DButton();
					nbtn->Create(GetParentControl(), rc, VISIBLE, nm.c_str(), nm.c_str(), btn->GetId());

					D2DWindow* d = nbtn->GetParentWindow();
					
					V4::MoveD2DControl(d, nbtn, 600, 300);*/

					


				};

				rcb1.Offset(120,0);
			}
			rcb1 = rcb;
			rcb.Offset(0, 40);
		}

	}

	xar_[test_]->Visible();
}




