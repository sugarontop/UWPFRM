#pragma once

#include "D2DUniversalControl.h"

namespace V4 {


	class D2DList : public D2DControls
	{
		public :
			D2DList(){}
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		private :
			void DrawList(D2DContext& cxt );
			void Parse(BSTR json);


			struct Item
			{
				std::wstring title;
				std::wstring value;
				int typ;
			};	

			std::vector<Item> ar_;

	};











};
