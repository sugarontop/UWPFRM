#include "pch.h"
#include "../libsrc/D2DUniversalControl.h"
#include "../libsrc/D2DTextbox.h"
#include "../libsrc/D2DIsland.h"
#include "../libsrc/D2DSliderControls.h"
#include "Entry.h"

using namespace concurrency;

void SoftSqueeze(D2DWindow* p, const std::vector<std::shared_ptr<RectSqueeze>>& ar, int milisec, int typ=0);


void add_control2(D2DControls* p, D2CoreTextBridge* imebridge)
{
	auto& caret = Caret::GetCaret();
	FRectFBoxModel rctext;

	rctext.SetRect(5, 5, FSizeF(100, 180));
	rctext.BoderWidth_ = 1.5f;
	rctext.Margin_.Set(0);
	rctext.Padding_.Set(0);

	D2DTextbox* txt = new D2DTextbox(*imebridge, D2DTextbox::MULTILINE, caret);
	txt->Create(p, D2DTextbox::Rect1(rctext), VISIBLE, NONAME);	

	txt->SetText( _u("Push right button."));

	FontInfo fi;
	fi.fontname = L"ƒƒCƒŠƒI";
	fi.height = 10;
	fi.weight = DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_MEDIUM;
	txt->SetFont(fi);

	//////////////////////////////////////////////
	//FRectFBoxModel rclist(5, 210, FSizeF(100, 200));

	//rclist.BoderWidth_ = 1.5f;
	//rclist.Margin_.Set(0);
	//rclist.Padding_.Set(0);

	//D2DListbox* ls1 = new D2DListbox();
	//ls1->Create(p, rclist, VISIBLE, NONAME, D2DListbox::TYP::SINGLELINE);


	//for (int i = 0; i < 50; i++)
	//{
	//	WCHAR cb[100];
	//	StringCbPrintf(cb, _countof(cb), _u("row=%d"), i);
	//	ls1->AddItem(cb);
	//}
}
void MoveObject(D2DControl* obj, float offx, float offy)
{
	auto toRect = obj->GetRect().OffsetRect(offx, offy);

	DWORD id = ::GetCurrentThreadId();

	create_task(create_async([obj, offx, offy,id]() {

		_ASSERT(id != ::GetCurrentThreadId());

		int step = 50;
		FRectF* prc = new FRectF[step];
		auto fromRect = obj->GetRect();

		float step_left = offx / step;
		float step_top = offy / step;
		float step_right = offx / step;
		float step_bottom = offy / step;


		for (int i = 0; i < step; i++)
		{
			prc[i] = fromRect;
			prc[i].left += step_left * i;
			prc[i].top += step_top * i;
			prc[i].right += step_right * i;
			prc[i].bottom += step_bottom * i;
		}
		auto w = obj->GetParentWindow();
		for (int i = 0; i < step; i++)
		{
			obj->SetRect(prc[i]);
			w->redraw();
			Sleep(24);
		}
		delete[] prc;


		})).then([obj, toRect, id]() {

			_ASSERT( id == ::GetCurrentThreadId());

			obj->SetRect(toRect);
			obj->GetParentWindow()->redraw();
		});
}


void OnEntry(D2DWindow* parent, FSizeF iniSz, D2CoreTextBridge* imebridge)
{

	try
	{		
		D2DMainWindow* main = dynamic_cast<D2DMainWindow*>(parent);
		D2DControls* ctrls1 = dynamic_cast<D2DControls*>(parent);
		_ASSERT(ctrls1);
		_ASSERT(main);

		D2DListbox* ls1 = new D2DListbox();

		main->imebridge_ = imebridge;

		main->SetBkColor(D2RGB(245,245,245));

		////


		FRectF rcisland(200, 100, FSizeF(200, 200));
		D2DIsland* land = new D2DIsland();
		land->Create(ctrls1, rcisland, VISIBLE, _u("D2DIsland"), -1);

		rcisland.SetRect(500, 300, FSizeF(200, 200));
		D2DIsland* land2 = new D2DIsland();
		land2->Create(ctrls1, rcisland, VISIBLE, _u("D2DIsland"), -1);


		rcisland.SetRect(800, 500, FSizeF(200, 200));
		D2DIsland* land3 = new D2DIsland();
		land3->Create(ctrls1, rcisland, VISIBLE, _u("D2DIsland"), -1);

		add_control2(land3, imebridge);


		FRectF rc(50, 500, FSizeF(100, 30));
		D2DButton* btn2 = new D2DButton();
		btn2->Create(ctrls1, rc, VISIBLE, _u("k¬"), NONAME);

		btn2->OnClick_ = [ctrls1, land,land2,land3](D2DButton* btn)
		{			
			land->ModeChange();
			land2->ModeChange();
			land3->ModeChange();

			std::vector<std::shared_ptr<RectSqueeze>> ar;

			RectSqueeze r1; r1.trc.SetRect(0,50,FSizeF(200,20)); r1.frc = land->GetRect(); r1.target = land->AnimeRect(1);
			RectSqueeze r2; r2.trc.SetRect(0,25,FSizeF(200,20)); r2.frc = land2->GetRect(); r2.target = land2->AnimeRect(1);
			RectSqueeze r3; r3.trc.SetRect(0,75,FSizeF(200,20)); r3.frc = land3->GetRect(); r3.target = land3->AnimeRect(1);
			ar.push_back(std::make_shared<RectSqueeze>(r1));
			ar.push_back(std::make_shared<RectSqueeze>(r2));
			ar.push_back(std::make_shared<RectSqueeze>(r3));

			SoftSqueeze(ctrls1->GetParentWindow(), ar, 800);

			
		};


		// ˜gF
		D2DLery* x = new D2DLery();
		rc.Set(3,150, 200,500);
		x->Create(ctrls1, rc, VISIBLE, _u("˜gF"), -1);


		D2DSliderButton* sb = new D2DSliderButton(6,true);
		rc.Set(250, 600, 60*6, 40);
		sb->Create(ctrls1, rc, 0, NONAME, -1);

		
		D2DButton* x1 = new D2DButton();
		rc.SetRect(500, 10, FSizeF(100, 30));
		x1->Create(ctrls1, rc, VISIBLE, _u("ˆÚ“®"),NONAME);

		x1->OnClick_=[land2](D2DButton* btn)
		{
			MoveObject(land2,500,100);
		};





		//rc.SetRect(0,0,1,50);
		//D2DSliderControls* bottomctrls = new D2DSliderControls();
		//bottomctrls->Create(ctrls1, rc, VISIBLE, _u("bottom_bar"), -1);

	}
	catch (std::wstring errmsg)
	{
		::OutputDebugString(errmsg.c_str());		
	}
}