#pragma once
#include "d2dcontext.h"
#include "d2dcommon.h"
#include "vectorstack.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"

#define  DRGB ColorPtr

namespace V4 {


class D2DSliderControls : public D2DControls
{
	public :
		D2DSliderControls();
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id);

		void DrawSqueeze();

	protected :
		FRectF rcMax_;
		float Height_;
		bool bOpen_;

};



};