#include "pch.h"
#include "D2DDevelop.h"
#include "D2DCommon.h"

using namespace V4;

#define TAB_HEIGHT 22
#define TAB_TRIM 2

D2DTabControls::D2DTabControls(){}

int D2DTabControls::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	if ( IsHide() && !IsImportantMsg(message))
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

			DrawTab( cxt );

			mat.Offset(0, (TAB_HEIGHT+TAB_TRIM));

			D2DRectFilter flt(cxt,FRectF(0,0,rc_.Size()));

			active_->WndProc(d,message,wp,lp);

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(lp);
			
			if (rc_.PtInRect(pt) && 0 <= pt.y && pt.y < TAB_HEIGHT)
			{
				pt.x -= rc_.left;

				GetParentControl()->SetCapture(this);

				int k = 0;
				for( auto& it : tabs_ )
				{
					if (it.rc.PtInRect(pt))
					{				
						active_ = it.control;
						active_idx_ = k;
						break;
					}
					k++;
				}
				
				
				d->redraw();
				ret = 1;
			}


		}
		break;
		case WM_LBUTTONUP:
		{
			if ( this == GetParentControl()->GetCapture())
			{
				GetParentControl()->ReleaseCapture();
				ret = 1;
			}
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

			if ( message == WM_D2D_INIT_UPDATE )
				Update();

			return 0;
		}
		break;
		case WM_LBUTTONDBLCLK:
		{
			FPointF pt = mat_.DPtoLP(lp);
			
			if (rc_.PtInRect(pt) && 0 <= pt.y && pt.y < TAB_HEIGHT)
			{
				pt.x -= rc_.left;

				int k = 0;
				for( auto& it : tabs_ )
				{
					if (it.rc.PtInRect(pt))
					{				
						active_ = it.control;
						if ( active_idx_ == k )
						{
							active_->WndProc(d,WM_D2D_TAB_DETACH, 0, nullptr);
							
							
							break;
						}
					}
					k++;
				}

				d->redraw();
				ret = 1;
			}
		}
		break;
	}

	if ( ret == 0 )
		ret = active_->WndProc(d,message,wp,lp);
	return ret;
}
void D2DTabControls::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	D2DWindow* win = pacontrol->GetParentWindow();
	InnerCreateWindow(win,pacontrol,rc,stat, name, -1);

	active_idx_ = 0;

}

FPointF LayoutCenter( FRectF rc, const FSizeF& textsize )
{
	FPointF pt = rc.CenterPt();
	pt.x -= textsize.width/2;
	pt.y -= textsize.height/2;
	return pt;
}


void D2DTabControls::Update()
{
	tabs_.clear();

	FRectF rc;
	auto& cxt = *(GetParentWindow()->cxt());
	auto wf = cxt.wfactory;
	int k = 0;
	tabs_.resize( controls_.size());

	rc.SetRect(0,2, 153, TAB_HEIGHT);
	for( auto& it : controls_ )
	{
		Tab& tab = tabs_[k++];

		std::wstring s = it->GetName();

		FSizeF sz = CreateTextLayout(cxt, s.c_str(), s.length(), &tab.text);
		rc.SetSize(sz.width+30, TAB_HEIGHT);

		tab.control = it;
		tab.rc = rc;
		tab.textsize = sz;

		rc.Offset(rc.Width(), 0);
	}

	active_ = controls_[0];

}
void D2DTabControls::DrawTab(D2DContext& cxt)
{
	auto br1 = CreateBrush( cxt, D2RGB(85,41,41));
	auto br2 = CreateBrush( cxt, D2RGB(255,240,208));

	FRectF rc(0,0,rc_.Width(),TAB_HEIGHT);
	cxt.cxt->FillRectangle(rc, br1);

	rc.SetRect(0,TAB_HEIGHT,rc_.Width(),TAB_HEIGHT+TAB_TRIM);
	cxt.cxt->FillRectangle(rc, br2);

	int k = 0;

	for( auto& it : tabs_ )
	{
		if ( active_idx_== k )
			cxt.cxt->FillRectangle(it.rc, br2);
		
		auto pt = LayoutCenter(it.rc, it.textsize);
		
		cxt.cxt->DrawTextLayout(pt, it.text, (active_idx_== k ? cxt.black:cxt.white) );

		k++;
	}
}

FRectF D2DTabControls::GetContentRect() const
{
	return FRectF(0,0, rc_.Width(), rc_.Height()-TAB_HEIGHT-TAB_TRIM);
}

