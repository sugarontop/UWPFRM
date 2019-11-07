#pragma once

#include "D2DUniversalControl.h"

namespace V4 {

	class D2DTextbox;
	class D2DCellsControlbar;

	// Excel•—‚Ì“ü—Í‰æ–Ê

	class D2DCells : public D2DControls
	{
		friend class D2DCellsControlbar;

		public :
			D2DCells(){}
			enum DT { NORMAL=0x1, EXCELLINE=0x2, EXCELTITLE=0x4};

			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, DT typ, LPCWSTR name, int local_id = -1);
		private :
			virtual void OnDXDeviceLost() override;
			virtual void OnDXDeviceRestored()  override;
			void Draw(D2DContext& cxt,INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp );

			virtual void UpdateScrollbar(D2DScrollbar* bar) override;
			int InnerDefWndScrollbarProc(D2DWindow* d, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam);
			
			FRectFBoxModel VScrollbarRect( const FRectFBoxModel& rc );
			void SetCanvasSize(float cx, float cy);
			void PreDraw();
			FRectF SelectCellF();
			
			
			int col_limit();
			int row_limit();

			float unit_w_, unit_h_;

			DT draw_mode_;

			enum MODE { NONE, MOVING,RESIZE,SCROLLBAR };
			MODE md_;

			struct Cell
			{
				FPointF pt;
				ComPTR<IDWriteTextLayout> txt;
			};

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


			struct Cell2
			{
				FPointF pt;
				FSizeF sz;
				ComPTR<IDWriteTextLayout> txt;
				std::wstring str;
				RowCol zrc;
			};

			std::vector<Cell> toptitle_;
			std::vector<Cell> lefttitle_;			
			std::vector<float> ypos_,xpos_;
			

			

			std::map<RowCol,Cell2> values_;

			RowCol sel_;
			D2DMat matd_;
			FSizeF scrollbar_off_;

			RowCol RCPos( FPointF pt );
			LPCWSTR CellName(RowCol rc);
			FPointF CellPos( LPCWSTR cellnm );
			RowCol CellRC(LPCWSTR cellnm);
			
			RowCol RCSelectPos(){ return sel_;}
			RowCol ZRCSelectPos();

			FPointF CellF(RowCol rc);
			void SetSelectCell( RowCol rc);
			LPCWSTR SelectCellName();

			void ActiveTextbox();
			
			D2DTextbox* tbox_;
			D2DCellsControlbar* bar_;
			std::shared_ptr<D2DControl> Vscbar_;
			int vrows_,vrowe_;
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