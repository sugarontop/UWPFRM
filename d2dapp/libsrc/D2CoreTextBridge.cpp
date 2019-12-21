#include "pch.h"
#include "D2DContext.h"

using namespace V4;

D2CoreTextBridge::D2CoreTextBridge():edcxt_(nullptr),target_(nullptr)
{

}
void D2CoreTextBridge::Set( Windows::UI::Text::Core::CoreTextEditContext^ cxt)
{	
	edcxt_ = cxt;
	edSelection_.StartCaretPosition = 0;
	edSelection_.EndCaretPosition = 0;
}
void D2CoreTextBridge::Activate(TextInfo* inf, void* target)
{
	target_ = target;
	info_ = inf;
	edcxt_->NotifyFocusEnter();

	Windows::UI::Text::Core::CoreTextRange rng;
	Windows::UI::Text::Core::CoreTextRange sel;

	sel.StartCaretPosition = info_->sel_start_pos;
	sel.EndCaretPosition = info_->sel_end_pos;


	edcxt_->NotifyTextChanged(rng,0,sel);
}
void D2CoreTextBridge::NotifyTextChanged( int cnt )
{				
	Windows::UI::Text::Core::CoreTextRange rng;
	Windows::UI::Text::Core::CoreTextRange sel;

	sel.StartCaretPosition = info_->sel_start_pos;
	sel.EndCaretPosition = info_->sel_end_pos;
	
	edcxt_->NotifyTextChanged(rng,cnt,sel);
}
void D2CoreTextBridge::UnActivate()
{
	target_ = nullptr;
	info_ = nullptr;
	edSelection_.StartCaretPosition = 0;
	edSelection_.EndCaretPosition = 0;
			

	edcxt_->NotifyFocusLeave();
}

void D2CoreTextBridge::SetCaret(int spos, int epos)
{
	Windows::UI::Text::Core::CoreTextRange sel;
	
	if ( epos < 0 )
		epos = spos;

	sel.StartCaretPosition = min(epos,spos);
	sel.EndCaretPosition = max(spos,epos);

	edcxt_->NotifySelectionChanged(sel);
}

void D2CoreTextBridge::SetNewSelection( Windows::UI::Text::Core::CoreTextRange& ed )
{
	edSelection_ = ed;
	info_->sel_start_pos = edSelection_.StartCaretPosition ;
	info_->sel_end_pos = edSelection_.EndCaretPosition ;
}

void D2CoreTextBridge::CompositionStarted(){}
void D2CoreTextBridge::CompositionCompleted()
{
	info_->decoration_typ = 0;
	info_->decoration_start_pos = info_->decoration_end_pos = 0;
	
}

Windows::UI::Text::Core::CoreTextRange D2CoreTextBridge::GetSelection()
{
	edSelection_.EndCaretPosition =	info_->sel_end_pos;
	edSelection_.StartCaretPosition = info_->sel_start_pos;

	return edSelection_;
}
void* D2CoreTextBridge::GetTarget()
{
	return target_;
}
/*
Index 3 and 7 is \n. The width is zero.

 ----+---+---+
 | 0 | 1 | 2 | 
-+---+---+---+
3| 4 | 5 | 6 |
-+---+---+---+
7| 8 | 9 |10 |
-+---+---+---+

Position 0 is left side of 0.
Position 3 is left side of 3.
Position 10 is left side of 10.

*/
void D2CoreTextBridge::UpdateTextRect( FSizeF rcMaxText )
{
	size_t len = info_->text.length();

	if (len == 0)
	{
		info_->clear();
		return;
	}

	ComPTR<IDWriteTextLayout> layout;		

	info_->wfac_->CreateTextLayout(info_->text.c_str(), len, info_->fmt_, rcMaxText.width, rcMaxText.height, &layout);
	info_->rcChars_ = CharFRects(len);

	const FRectF* prc = info_->rcChars_.get();
	
	info_->line_cnt = 1;
	//info_->rcCharCnt = len;
	float prtop = 0;

	for (UINT i = 0; i < len; i++)
	{
		DWRITE_HIT_TEST_METRICS tm;
		float x1 = 0, y1 = 0;
		layout->HitTestTextPosition(i, false, &x1, &y1, &tm);
		
		FRectF rc( tm.left, tm.top, tm.left+tm.width, tm.top+tm.height);
		
		if ( rc.Width() == 0 && info_->text[i] == L'\n' )
		{
			rc.Offset(0,rc.Height());
			rc.left = rc.right = 0;
		}
	
		((FRectF*)prc)[i] = rc;

		if ( prtop < rc.top )
		{
			info_->line_cnt++;
			prtop = rc.top;
		}							
	}
}


