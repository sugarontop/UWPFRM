#pragma once

#include "D2DUniversalControl.h"

namespace V4 {

	class D2DTextbox;
	class D2DCellsControlbar;

	class D2DCells : public D2DControls
	{
		friend class D2DCellsControlbar;

		public :
			D2DCells(){}
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		private :
			void Draw(D2DContext& cxt,INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp );
			void PreDraw();
			FRectF SelectCellF();
			
			int col_limit();
			int row_limit();

			float unit_w_, unit_h_;

			enum DT { NORMAL=0x1, EXCELLINE=0x2, EXCELTITLE=0x4};

			DT draw_mode_;

			struct Cell
			{
				FPointF pt;
				ComPTR<IDWriteTextLayout> txt;
			};
			struct Cell2
			{
				FPointF pt;
				FSizeF sz;
				ComPTR<IDWriteTextLayout> txt;
				std::wstring str;
			};

			std::vector<Cell> toptitle_;
			std::vector<Cell> lefttitle_;			
			std::vector<float> ypos_,xpos_;
			

			struct RowCol
			{
				RowCol():x(0),y(0){}
				int x, y; // 0start

				bool operator <(const RowCol& rc) const
				{
					if ( y == rc.y && x < rc.x ) 
						return true;
					else if ( y < rc.y ) 
						return true;
					else
						return false;
				}
			};

			std::map<RowCol,Cell2> values_;

			RowCol sel_;
			D2DMat matd_;

			RowCol RCPos( FPointF pt );
			LPCWSTR CellName(RowCol rc);
			FPointF CellPos( LPCWSTR cellnm );
			RowCol CellRC(LPCWSTR cellnm);

			FPointF CellF(RowCol rc);
			void SetSelectCell( RowCol rc);
			LPCWSTR SelectCellName();

			D2DTextbox* tbox_;
			D2DCellsControlbar* bar_;
			
	};



	class D2DCellsControlbar : public D2DControls
	{
		public :
			D2DCellsControlbar(){}
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		private :
			
			D2DTextbox* tbox_;

	};

};