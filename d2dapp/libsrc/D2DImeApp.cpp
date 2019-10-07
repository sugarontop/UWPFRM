#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DTextbox.h"

using namespace V4;
Windows::Foundation::Rect ClientToScreen(FRectF rc);

void App_OnInputLanguageChanged(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextServicesManager^ sender, Platform::Object^ args)
{
	Windows::Globalization::Language^ lang = sender->InputLanguage;

}
void App_OnTextUpdating(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextTextUpdatingEventArgs^ args)
{
	imeBridge.SetNewSelection(args->NewSelection);

	LPCWSTR c = args->Text->Data();

	size_t append_len = wcslen(c);

	size_t ys = args->Range.StartCaretPosition;
	size_t ye = args->Range.EndCaretPosition;


	auto str3 = str_remove(imeBridge.info_->text, ys, ye);

	for (size_t i = 0; i < append_len; i++)
	{
		if (ys + i < str3.length())
			str3 = str_append(str3, ys + i, c[i]);
		else
		{
			str3 += c;
			break;
		}
	}

	imeBridge.info_->text = str3;
	imeBridge.info_->decoration_end_pos = min((int)str3.length(), imeBridge.info_->decoration_end_pos);

	auto& rc = imeBridge.info_->rcTextbox;
	imeBridge.UpdateTextRect(rc.Size());

	if (imeBridge.info_->ontext_updated_)
		imeBridge.info_->ontext_updated_();

//	m_main->redraw_ = true;

}

void App_OnFormatUpdating(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextFormatUpdatingEventArgs^ args)
{
	//TRACE( L"OnFormatUpdating " );

	int typ = (int)args->UnderlineType->Value;


	if (imeBridge.info_)
	{
		imeBridge.info_->decoration_start_pos = args->Range.StartCaretPosition;
		imeBridge.info_->decoration_end_pos = args->Range.EndCaretPosition;
		imeBridge.info_->decoration_typ = typ;

	}

	//::OutputDebugString( V4::Format( L"%d %d typ:%d\n", args->Range.StartCaretPosition, args->Range.EndCaretPosition, typ).c_str() );
	//m_main->redraw_ = true;
}

void App_OnCompositionStarted(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextCompositionStartedEventArgs^ args)
{
	// start convert to jp.

	imeBridge.CompositionStarted();

	//TRACE( L"OnCompositionStarted\n" );

}
void App_OnCompositionCompleted(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextCompositionCompletedEventArgs^ args)
{
	// finished converting.

	imeBridge.CompositionCompleted();
	//m_main->redraw_ = true;

	//TRACE( L"OnCompositionCompleted\n" );

}

void App_OnFocusRemoved(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Platform::Object^ args)
{
	//TRACE(L"OnFocusRemoved\n");
}

void App_OnLayoutRequested(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextLayoutRequestedEventArgs^ args)
{
	// candidate‚Ì•\Ž¦ˆÊ’u

	if (imeBridge.info_)
	{
		auto rc = imeBridge.info_->rcIme; //rcTextbox;

		int spos = args->Request->Range.StartCaretPosition;
		int epos = args->Request->Range.EndCaretPosition;

		if (0 < spos && spos < imeBridge.info_->rcChar().count())
		{
			FRectF rcChar = imeBridge.info_->rcChar().get(spos - 1);

			FSizeF sz(rcChar.right, 0);

			D2DMat m(imeBridge.info_->mat);
			auto szDev = m.LPtoDP(sz);


			rc.Offset(szDev.width, 0);
		}

		args->Request->LayoutBounds->ControlBounds = ClientToScreen(imeBridge.info_->rcTextbox);
		args->Request->LayoutBounds->TextBounds = ClientToScreen(rc);// ime candidate window position

		//TRACE(L"OnLayoutRequested %d-%d  top=%f,bottom=%f\n", args->Request->Range.StartCaretPosition, args->Request->Range.EndCaretPosition, rc.top, rc.bottom);
	}
}

void App_OnSelectionRequested(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextSelectionRequestedEventArgs^ args)
{
	args->Request->Selection = imeBridge.GetSelection();

	//TRACE( L"OnSelectionRequested %d \n", args->Request->Selection );
}

void App_OnSelectionUpdating(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextSelectionUpdatingEventArgs^ args)
{
	imeBridge.SetNewSelection(args->Selection);

	//TRACE( L"OnSelectionUpdating\n" );
}
void App_OnTextRequested(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextTextRequestedEventArgs^ args)
{
	if (imeBridge.info_)
	{
		std::wstring& s = imeBridge.info_->text;

		int start = args->Request->Range.StartCaretPosition;
		int end = min(args->Request->Range.EndCaretPosition, (int)s.length());

		std::wstring s2 = s.substr(start, end - start);

		args->Request->Text = ref new Platform::String(s2.c_str());


		//TRACE( L"OnTextRequested, (%d-%d), %s\n", start,end, s2.c_str() );
	}
}
Windows::Foundation::Rect ClientToScreen(FRectF rc)
{
	Windows::Foundation::Rect rcc = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Bounds;

	rc.Offset(rcc.Left, rcc.Top);

	Windows::Foundation::Rect r;
	r.X = rc.left;
	r.Height = rc.bottom - rc.top;
	r.Y = rc.top;
	r.Width = rc.right - rc.left;
	return r;
}