#include "pch.h"
#include "../D2DUniversalControlBase.h"
#include "../D2DWindowMessage.h"
#include "../D2DCommon.h"
#include "TightRopeCell.h"


using namespace V4;

#define ROWHEIGHT 32.0f

TightRopeCell::TightRopeCell():row_cnt(1),col_cnt(1),next_(0),prev_(0)
{
	
}
void TightRopeCell::Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id)
{
	InnerCreateWindow(pacontrol, rc, stat, name, local_id);
}
void TightRopeCell::SetNext(TightRopeCell* next)
{
	next_ = next;
	next->prev_ = this;
}

static void DrawText(D2DContext& cxt, std::wstring& s, const FRectF& rc, ID2D1SolidColorBrush* clr)
{
	size_t len = s.length();
	if ( len )
		cxt.cxt->DrawText(s.c_str(), len, cxt.cxtt.textformat, rc, clr );
}

void Offset( FRectF& rc, float cx, float cy, float newwidth )
{
	rc.Offset(cx,cy);
	rc.right = rc.left + newwidth;

}

#define PADDING 2

int TightRopeCell::WndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
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

			

			cxt.cxt->DrawRectangle(rc, cxt.yellow);

			mat.Offset(rc_.left, rc_.top);

			rc.SetRect(0,0,0, ROWHEIGHT);

			ComPTR<ID2D1SolidColorBrush> clr[] = { cxt.black, cxt.white, cxt.bluegray,cxt.gray };


			for (auto it : xcells_)
			{
				auto&xc = it.second;
				rc = xc.rc;
				cxt.cxt->FillRectangle(xc.rc, clr[xc.fill_clridx] );
				DrawText(cxt, xc.value, rc.OffsetRect(PADDING, PADDING), clr[xc.txt_clridx]);

			}

			
			DefPaintWndProc(d, message, wp, lp);

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_TEXTBOX_PRE_CHANGE:
		{
			WParameter& _wp = *(WParameter*)wp;

			//static std::wstring value = L"000002";

			//if (value == (LPCWSTR)_wp.prm)
			//{
			//	ret = 0;

			//}
			//else
			//{
			//	ret = 1;

			//	_wp.prm = (LPVOID)value.c_str(); // Œ³‚É–ß‚·
			//}


		}
		break;
		case WM_D2D_TEXTBOX_CHANGED:
		{
			WParameter& _wp = *(WParameter*)wp;

			auto& xc = xcells_[active_];
			xc.value = (LPCWSTR)_wp.prm;


			active_.row++;

			auto p = controls_[0].get();
			Active(active_.row, active_.col, dynamic_cast<D2DTextbox*>(p));
			ret = 1;
		}
		break;
		case WM_D2D_TEXTBOX_PUSHED_OPTIONKEY:
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			if ( arg->VirtualKey == Windows::System::VirtualKey::Enter )
			{
				
				auto p = controls_[0].get();

				//Active(2,1, dynamic_cast<D2DTextbox*>(p));

				//this->SetCapture(p);
				dynamic_cast<D2DTextbox*>(p)->Activate();

				ret = 1;
			}
		}
		break;

	}


	if (ret == 0)
		ret = D2DControls::DefWndProc(d, message, wp, lp);


	

	return ret;
}

void TightRopeCell::Active(WORD row, WORD col, D2DTextbox* tx)
{
	//_ASSERT(row < cells_.size());
	auto old = tx->GetParentControl();

	if ( old != this )
	{
		auto ttx = old->Detach(tx);
		this->controls_.push_back(ttx);
	}

	auto& c = xcells_[CellKey(row, col)];

	FRectFBoxModel trc(c.rc);
	trc.Padding_.l = PADDING;
	trc.Padding_.t = PADDING;
	   
	tx->SetRect(trc); // title‚Ì‰¡‚ªtextbox
	tx->SetText(c.value.c_str());
}

void TightRopeCell::SetText(WORD row, WORD col, LPCWSTR tx)
{
	auto& c = xcells_[CellKey(row, col)];
	c.value = tx;

}

void TightRopeCell::AddRow1(const std::wstring& title, UINT row )
{
	rc_.bottom = rc_.top+ROWHEIGHT*(row);
	
	XCell xc;
	
	xc.bEditable = false;
	xc.col=0;
	xc.row = row;
	xc.coltyp = STR;
	xc.value = title;

	xc.fill_clridx=2;
	xc.txt_clridx=3;
	xc.rc = FRectF(0,0,100,ROWHEIGHT); xc.rc.Offset(0, ROWHEIGHT*row);
	
	
	auto a = CellKey(xc.row, xc.col);
	
	xcells_.insert(std::pair<CellKey, XCell>(a, xc));

	xc.bEditable = true;
	xc.col = 1;
	xc.row = row;
	xc.coltyp = STR;
	xc.value = L"";
	xc.fill_clridx = 3;
	xc.txt_clridx = 0;
	xc.rc = FRectF(100, 0, FSizeF(150, ROWHEIGHT)); xc.rc.Offset(0, ROWHEIGHT * row);

	a = CellKey(xc.row, xc.col);
	
	xcells_.insert(std::pair<CellKey, XCell>(a, xc));

	

	xc.bEditable = false;
	xc.col = 2;
	xc.row = row;
	xc.coltyp = STR;
	
	xc.fill_clridx = 3;
	xc.txt_clridx = 2;
	xc.rc = FRectF(250, 0, FSizeF(200, ROWHEIGHT)); xc.rc.Offset(0, ROWHEIGHT * row);

	a = CellKey(xc.row, xc.col);
	
	xcells_.insert(std::pair<CellKey, XCell>(a, xc));


	active_ = CellKey(0,1);
}