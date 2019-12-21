#pragma once

#include "D2DUniversalControlBase.h"

namespace V4 {


	class D2DRibonMenu : public D2DControls
	{
		public:
			D2DRibonMenu();
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id);
			
			void ShowPictureItem();
			void HidePictureItem();

			void AddMenuItem( LPCWSTR item, D2DControl* ctrl);

		private:
		
			enum TYP {NONE,SELECT,ACTIVE};
			struct Item
			{
				ComPTR<IDWriteTextLayout> layout;
				FRectFBoxModel rc;		
				D2DControl* ctrls;
			};

			int select_idx_;
			int active_idx_;
		
			
			std::vector<Item> items_;
			FSizeF szWindow_;
		
	};

	class D2DRibonMenuPictureItem : public D2DControls
	{
		public :
			D2DRibonMenuPictureItem();
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id);
			

			
			FSizeF off_move_;

	};

};
