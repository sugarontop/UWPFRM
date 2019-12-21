#pragma once

#include "D2DUniversalControlBase.h"

namespace V4 {

	class D2DFooterbar : public D2DControls
	{
		public:
			D2DFooterbar();
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id);
		private:
	};
};


