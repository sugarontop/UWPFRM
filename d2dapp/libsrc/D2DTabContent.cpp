#include "pch.h"
#include "D2DUniversalControl.h"
//#include "D2DUniversalControlBase.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DTab.h"
#include "D2DIsland.h"

using namespace V4;
using namespace concurrency;

void MoveObject(D2DControl* obj, float offx, float offy);

#undef LOGPT
#define LOGPT(xpt,wp) FPointF xpt = mat1.DPtoLP(*(FPointF*)(wp));

void drawinfo(D2DContext& cxt, float x, float y, std::wstring & str )
{
	cxt.cxt->DrawText(str.c_str(), str.length(), cxt.textformat, FRectF(x,y,FSizeF(1000,26)), cxt.black );
}




int xfunc (D2DIsland* p1, D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	int ret = 0;

	//if (IsHide() && !IsImportantMsg(message))
	//	return 0;

	switch (message)
	{
		case WM_PAINT:
		{
			CXTM(d)

			auto mat1 = p1->GetMat(); 

			D2DMatrix mata(mat1,cxt);

			auto rc = p1->GetRect();

			mata.PushTransform();
			mata.Offset(rc); 
		
			FRectF rc1(0,0,900,700);



			DrawFramelikeMFC(cxt, rc1, cxt.white);
			
			mata.Offset(20,10);
			drawinfo(cxt, 0, 0, V4::Format(L"pos=(%0.02f %0.02f) sz=(%0.02f %0.02f)", rc.left, rc.top, rc.Size().width, rc.Size().height ));
			drawinfo(cxt, 0, 26, V4::Format(L"nm=%s", p1->GetName().c_str()));
			drawinfo(cxt, 0, 52, V4::Format(L"parent nm=%s", p1->GetParentControl()->GetName().c_str()));
			




			mata.PopTransform();

			return 0;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			auto mat1 = p1->GetMat();
			LOGPT(pt3, wp)
			auto rc = p1->GetRect();

			if ( rc.PtInRect(pt3))
			{				
				p1->GetParentControl()->SetCapture(p1);
				ret = 1;
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			auto mat1 = p1->GetMat();
			LOGPT(pt3, wp)
			auto rc = p1->GetRect();

			if (p1 == p1->GetParentControl()->GetCapture())
			{
				FPointF pt(lp);

				WParameterMouse* prvm = (WParameterMouse*)wp;

				auto offx = pt.x - prvm->move_ptprv.x;
				auto offy = pt.y - prvm->move_ptprv.y;

				auto rc = p1->GetRect();
				rc.Offset(offx, offy);
				p1->SetRect(rc);

				ret = 1;
				d->redraw();

			}
		}
		break;
		case WM_LBUTTONUP:
		{
			auto mat1 = p1->GetMat();
			LOGPT(pt3, wp)
			auto rc = p1->GetRect();

			if (p1 == p1->GetParentControl()->GetCapture())
			{
				p1->GetParentControl()->ReleaseCapture();
				ret = 1;

				//p1->outproc_ = nullptr;
			}
		}
		break;
	}

	return ret;
}



int xfunc2(D2DIsland* p1, D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	int ret = 0;

	//if (IsHide() && !IsImportantMsg(message))
	//	return 0;

	switch (message)
	{
		case WM_PAINT:
		{
			CXTM(d)

			auto mat1 = p1->GetMat();

			D2DMatrix mata(mat1, cxt);

			auto rc = p1->GetRect();

			mata.PushTransform();
			mata.Offset(10, 100);

			FRectF rc1(0, 0, 80, 30);
			

			DrawFramelikeMFC(cxt, rc1, cxt.white);

			drawinfo(cxt, 0, 0, V4::Format(L"nm=%s", p1->GetName().c_str()));
				   


			mata.PopTransform();

			return 0;
		}
		break;
		case WM_D2D_ATTACH:
		{
			
		}
		break;
		case WM_D2D_DETACH:
		{

		}
		break;
		case WM_MOUSEMOVE:
		{
			auto rc = p1->GetRect();
			


		}
		break;

	}

	return ret;
}



D2DTabContent::D2DTabContent()
{

}
int D2DTabContent::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	int ret = 0;

	if (IsHide() && !IsImportantMsg(message))
	{

	}
	else
	{
		switch (message)
		{
			case WM_PAINT:
			{
				CXTM(d)

				mat_ = mat.PushTransform();

				{
					D2DRectFilter fl(cxt, rc_);

					cxt.cxt->FillRectangle(rc_.InflateRect(-1, -1), cxt.ltgray);
					//cxt.cxt->DrawRectangle(rc_, clr[clridx_], 2);

					mat.Offset(rc_.left, rc_.top);

					FRectF rc(0,0,200,30);
					cxt.cxt->DrawText(L"D2DTabContent", 13, cxt.textformat, rc, cxt.black );


					DefPaintWndProc(d, message, wp, lp);
				}

				mat.PopTransform();

				return 0;
			}
			break;

			case WM_KEYDOWN :
			{
				Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
				switch (arg->VirtualKey)
				{
					case Windows::System::VirtualKey::Escape:
					{
						int a = 0;

					}
					break;
				}
			}
			break;
			case WM_LBUTTONDOWN:
			{
				FPointF ptd(lp);

				auto mat1 = GetMat();
				LOGPT(pt3, wp)
				
				if (rc_.PtInRect(pt3))
				{
					d->SetFocus(this);
					//ret = 1;
				}
			}
			break;
			case WM_D2D_SETFOCUS:
			{
				
				D2DControl* pdc = (D2DControl*)wp;
				_ASSERT(pdc == this);
				stat_ |= STAT::FOCUS;
				ret = 1;
			}
			break;
			case WM_D2D_KILLFOCUS:
			{
				stat_ &= ~STAT::FOCUS; 
				ret = 1;
			}
			break;

			case WM_SIZE:
			case WM_D2D_INIT_UPDATE:
			{
				FSizeF sz = GetParentControl()->GetRect().Size();
				rc_.SetRect(0, 0, sz);
			}
			break;			
		}

		if (ret == 0 && WM_PAINT != message )
			ret = D2DControls::DefWndProc(d, message, wp, lp);
	
	}
	return ret;


}



void D2DTabContent::Create(D2DControls* pacontrol, int stat, LPCWSTR name, int local_id)
{
	FRectF rc(0,0,-1,-1);
	InnerCreateWindow(pacontrol, rc, stat, name, local_id);

	md_ = 0;

}

