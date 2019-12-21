#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DTextbox.h"
#include "D2DRibonMenu.h"

using namespace V4;
using namespace concurrency;



D2DRibonMenu::D2DRibonMenu():select_idx_(-1), active_idx_(-1)
{

}
int D2DRibonMenu::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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

			ComPTR<ID2D1SolidColorBrush> clr[] = { cxt.blue, cxt.red, cxt.gray };

			{
				cxt.cxt->FillRectangle(rc_, cxt.gray);
				
				int i=0;
				for(auto& it : items_)
				{
					FPointF pt = it.rc.GetContentRect().LeftTop();

					if ( i == select_idx_ )
					{
						cxt.cxt->FillRectangle(it.rc, cxt.bluegray);
						cxt.cxt->DrawTextLayout(pt, it.layout, cxt.yellow );
					}
					else if ( i == active_idx_ )
					{
						cxt.cxt->FillRectangle(it.rc, cxt.ltgray2);
						cxt.cxt->DrawTextLayout(pt, it.layout, cxt.black);

					}
					else
						cxt.cxt->DrawTextLayout(pt, it.layout, cxt.white);
					i++;
				}
				mat.Offset(0, rc_.Height());

				FRectF xrc(0,0,szWindow_);
				D2DRectFilter fil(cxt, xrc);

				DefPaintWndProc(d, message, wp, lp);
			}

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_INIT_UPDATE:
		case WM_SIZE:
		{
			szWindow_ = *(FSizeF*)wp;
			rc_.SetWidth(szWindow_.width);
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
				int i = 0;
				int idx = -1;
				for (auto& it : items_)
				{
					if ( it.rc.PtInRect(pt3))
					{
						idx = i;
						break;
					}
					i++;
				}

				if ( idx != select_idx_ )
				{
					select_idx_ = idx;
					active_idx_ = -1;
					d->redraw();

				}

				ret = 1;
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			LOGPT(pt3, wp);
			if (rc_.PtInRect(pt3))
			{
				int i = 0;
				int idx = -1;
				for (auto& it : items_)
				{
					if (it.rc.PtInRect(pt3))
					{
						idx = i;
						break;
					}
					i++;
				}

				if (idx == select_idx_)
				{
					active_idx_ = idx;
					select_idx_ = -1;
					d->redraw();

					ShowPictureItem();
				}
				ret = 1;
			}
		}
		break;
		case WM_RBUTTONDOWN:
		{
			int a = 0;
		}
		break;
		case WM_KEYDOWN:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			switch (arg->VirtualKey)
			{
				case Windows::System::VirtualKey::Escape:
					this->ReleaseCapture();				
				break;
			}
		}
		break;
	}
	if (ret == 0)
		ret = D2DControls::DefWndProc(d, message, wp, lp);

	return ret;

}
void D2DRibonMenu::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol, rc, stat, name, local_id);

}

//void D2DRibonMenu::SetMenuItem(const std::vector<Ribon>& titles)

int ggg = 0;
void D2DRibonMenu::AddMenuItem(LPCWSTR itemnm, D2DControl* ctrl)
{
	auto tf2 = GetParentWindow()->cxt()->cxtt.textformat;
	auto wf = GetParentWindow()->cxt()->cxtt.wfactory;

	FPointF pt(0,0);
	Item item;
	
	if (HR(wf->CreateTextLayout(itemnm, (UINT32)wcslen(itemnm), tf2, 1000, 1000, &item.layout)))
	{
		DWRITE_TEXT_METRICS tm;
		item.layout->GetMetrics(&tm);

		float xxx = 100 * ggg++;
			
		FRectFBoxModel rc( xxx, 0, FSizeF(0,0));
		rc.Padding_.SetRL(10);
		rc.Padding_.SetTB(4);
		rc.SetContentSize(FSizeF(tm.width, tm.height));
			

		if (ctrl == nullptr )
		{
			auto c = new D2DStatic();
			c->Create(this,FRectF(0,0,100,100), VISIBLE, _u("ˆÀS") );				 
			item.ctrls = c;
		}
		else
		{
			item.ctrls = ctrl;
		}

		_ASSERT(item.ctrls->GetParentControl() == this );
			
		item.rc = rc;
		items_.push_back(item);

		auto target = this->Detach(item.ctrls);
			
		FRectF rc1(0,0,100,93);
		D2DRibonMenuPictureItem* pic = new D2DRibonMenuPictureItem();

		int lcid = controls_.size();

		pic->Create(this, rc1,NONE,_u("D2DRibonMenuPictureItem"), lcid);			
		pic->Attach(target);
	}
	
}

void D2DRibonMenu::HidePictureItem()
{	
	active_idx_ = -1;
	select_idx_ = -1;
	
	auto p2 = dynamic_cast<D2DRibonMenuPictureItem*>(GetCapture());

	if (p2)
	{
		ReleaseCapture(p2);		
		p2->Hide();

		auto p = GetCapture();
		_ASSERT(p==this);
		//ReleaseCapture(this);

		ReleaseCapture(); // all free
	}

}




void D2DRibonMenu::ShowPictureItem()
{
	if ( 0 <= active_idx_ && active_idx_ < controls_.size())
	{
		for(auto& it: controls_) it->Hide();

		D2DControl* p=nullptr;
		for (auto& it : controls_)
		{
			if (active_idx_ == it->GetId())
			{
				p = it.get();
								
				p->Visible();

				auto rc = p->GetRect();
				rc.SetWidth(rc_.Width());
				p->SetRect(rc);

				D2DRibonMenuPictureItem* p1 = (D2DRibonMenuPictureItem*)p;
				p1->off_move_.height = -rc.Height();
				
				MoveMatrixY( GetParentWindow(), &p1->off_move_.height, rc.Height());

				break;
			}
		}
		
		auto p2 = dynamic_cast<D2DRibonMenuPictureItem*>(GetCapture());

		if ( p2 )
		{
			p2->Hide();
			ReleaseCapture(p2);
		}
		
		SetCapture(p); // controls_“à‚Ì‡”Ô‚ª•ÏX‚³‚ê‚é‚Ì‚Å’ˆÓ

	}

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


D2DRibonMenuPictureItem::D2DRibonMenuPictureItem()
{

}
int D2DRibonMenuPictureItem::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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

			mat.Offset(rc_.left, rc_.top+ off_move_.height);

			{
				cxt.cxt->FillRectangle(rc_, cxt.ltgray2);

				WCHAR s[10];
				StringCbPrintf(s,10,L"%d", this->id_);
				FRectF rc(10,10,200,30);
				cxt.cxt->DrawText(s,wcslen(s), cxt.cxtt.textformat, rc, cxt.white);

				

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
			rc_.SetWidth(sz.width);
		}
		break;
		case WM_LBUTTONDOWN:
		{
			
			LOGPT(pt3, wp)

			if (rc_.PtInRect(pt3))
			{
			
			}
			else if ( this == parent_control_->GetCapture())
			{				
				D2DRibonMenu* p = dynamic_cast<D2DRibonMenu*>(parent_control_);
				_ASSERT(p);
				p->HidePictureItem();

				ret = 1;
			}


		}
		break;

	}

	if (ret == 0)
		ret = D2DControls::DefWndProc(d, message, wp, lp);

	return ret;

}
void D2DRibonMenuPictureItem::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol, rc, stat, name, local_id);

}