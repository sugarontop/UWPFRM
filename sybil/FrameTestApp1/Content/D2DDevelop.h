#pragma once

#include "D2DUniversalControl.h"
#include "higgsjson.h"
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
		

	class D2DVerticalbarControls : public D2DControls
	{
		public :
			D2DVerticalbarControls(){}
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, D2D1_COLOR_F* clr, int clrcnt );
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)  override;
		protected :
			void OnPaint(D2DContext& cxt);
			

			D2D1_COLOR_F clr_[3];

	};
	class D2DTextbox;
	class D2DDropDownListbox;

	class D2DPropertyControls : public D2DControls
	{
		public :
			D2DPropertyControls(){}
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)  override;

			void Load(const BSTR json );

		protected :
			void DrawList(D2DContext& cxt);
			void PreDraw();
			

			D2D1_COLOR_F clr_[3];
			enum TYP { TEXT,LIST };

			struct Row
			{
				bool readonly;
				std::wstring title;
				std::wstring value;
				
				ComPTR<IDWriteTextLayout> ctitle;
				ComPTR<IDWriteTextLayout> cvalue;

				FRectF rc;
				TYP typ;
				HiggsJson::Higgs items;
				int selectidx;
			};

			std::vector<Row> ar_;
			int select_row_;
			D2DMat matd_;
			D2DTextbox* tbox_;
			D2DDropDownListbox* ls_;
			BSTR data_;

	};





};
