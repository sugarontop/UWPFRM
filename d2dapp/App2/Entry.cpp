#include "pch.h"
#include "../libsrc/D2DUniversalControl.h"
#include "../libsrc/D2DTextbox.h"
#include "Entry.h"

void OnEntry(D2DWindow* parent, FSizeF iniSz, D2CoreTextBridge* imebridge)
{
	try
	{		
		D2DMainWindow* main = dynamic_cast<D2DMainWindow*>(parent);
		D2DControls* ls = dynamic_cast<D2DControls*>(parent);
		_ASSERT(ls);
		_ASSERT(main);

		main->imebridge_ = imebridge;

		

		auto& caret = Caret::GetCaret();
		FRectF rctext(120, 100, FSizeF(400, 26));
		D2DTextbox* tx = new D2DTextbox(*imebridge, caret);
		tx->Create(ls, rctext, VISIBLE, _u("dumy"));
		tx->SetText(_u("Textbox"));


		FRectF rc1(120, 200, FSizeF(100, 60));
		D2DButton* btn = new D2DButton();
		btn->Create(ls, rc1, VISIBLE, L"Click me", NONAME);
		btn->OnClick_ = [tx](D2DButton* b) {

			auto s = tx->GetText();
			s += _u("Pushed!");
			tx->SetText( s.c_str());

		};


	}
	catch (std::wstring errmsg)
	{
		::OutputDebugString(errmsg.c_str());		
	}
}