#pragma once

#include "D2DUniversalControlBase.h"

namespace V4 {

	class D2DTabContent : public D2DControls
	{
		public:
			D2DTabContent();
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, int stat, LPCWSTR name, int local_id);

			int md_;

	};



}
