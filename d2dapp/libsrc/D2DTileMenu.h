
#pragma once

#include "D2DUniversalControlBase.h"

namespace V4 {

	class D2DTileMenu : public D2DControls
	{
		public:
			D2DTileMenu();
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id);


			void Test(int pagecnt );

			
		private:

			int pagecnt_;
			int test_;

			std::vector<D2DControls*> xar_;
			
	};
};


