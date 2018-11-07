#include "pch.h"
#include "D2DUniversalControlBase.h"
#include "D2DTextbox.h"


using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;

namespace V4
{

void Blink(Windows::System::Threading::ThreadPoolTimer^ timer)
{
	int a = Caret::GetCaret().bShow_;
	
	if ( a == 1 )
		a = -1;
	else if ( a == -1 )
		a = 1;
	else if ( a == 0 )
		timer->Cancel();

	Caret::GetCaret().bShow_ = a;
}

Caret::Caret()
{
	ti_ = nullptr;
	bShow_ = 0;
	is_start_change_ = 0;
}

Caret& Caret::GetCaret()
{
	static Caret ca;
	return ca;
}

void Caret::Activate( TextInfo& ti )
{
	ti_ = &ti;
	bShow_ = 1;
	is_start_change_ = false;

	Windows::Foundation::TimeSpan blink_tm;
	blink_tm.Duration = 10*1000*500;

	if ( timer_ == nullptr )
		timer_ = ThreadPoolTimer::CreatePeriodicTimer( ref new TimerElapsedHandler(Blink), blink_tm );
}

void Caret::UnActivate()
{
	ti_ = false;
	bShow_ = 0;

	if ( timer_ )
		timer_->Cancel();

	is_start_change_ = 0;
	timer_ = nullptr;
}



void Caret::ShowCaret()
{
	bShow_ = 1;
}
void Caret::HideCaret()
{
	bShow_ = 0;
}
		
bool Caret::Draw(D2DContext& cxt )
{
	if ( bShow_ == 0 || ti_ == nullptr ) return false;
	else if ( bShow_ == -1 ) return true;
		
	auto rc = GetCaretRect();

	D2DMatrix mat(cxt);

	mat.PushTransform();
			
	mat.NewTransform(ti_->mat);
	mat.Offset( ti_->rcTextboxLog.left, ti_->rcTextboxLog.top );
	
	cxt.cxt->FillRectangle( rc, cxt.black );
	
	
	mat.PopTransform();
	return true;
}
FRectF Caret::GetCaretRect()
{
	FRectF rc(0,0,2,ti_->line_height);
	int pos = ( is_start_change_ == 1 ? ti_->sel_start_pos : ti_->sel_end_pos );
	
	if ( pos > 0  )
	{
		rc = ti_->rcChar().get()[ pos-1 ]; 		
		rc.Offset( rc.Width(), 0 );					
		
		rc.right = rc.left + 2;
	}
	
	return rc;
}












};