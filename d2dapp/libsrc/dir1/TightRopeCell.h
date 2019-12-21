#pragma once
#include "../d2dcontext.h"
#include "../d2dcommon.h"
#include "../vectorstack.h"
#include "../d2dtextbox.h"
#include "../D2DWindowMessage.h"

#define  DRGB ColorPtr

namespace V4 {

	class TightRopeCell : public D2DControls
	{
		public:
			TightRopeCell();

			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id);

			void SetNext(TightRopeCell* next);

			enum COLTYP { STR,MONEY,DATE,DATETIME };

			

			struct XCell
			{
				std::wstring value;
				COLTYP coltyp;
				WORD col, row;
				bool bEditable;

				FRectF rc;
				WORD fill_clridx;
				WORD txt_clridx;
			};

			void AddRow1( const std::wstring& title, UINT row);
			void Active( WORD row, WORD col, D2DTextbox* tx );
			void SetText(WORD row, WORD col, LPCWSTR tx);

		protected :
			WORD row_cnt, col_cnt;
			

			struct CellKey
			{
				WORD row,col;

				CellKey():row(0),col(0){}
				CellKey(WORD r, WORD c):row(r),col(c){};

				bool operator < (const CellKey& a) const
				{
					_ASSERT( &a != this );
					if (a.row == row && a.col < col) return true; 
					else if (a.row < row ) return true;
					return false;					
				}
			};

			std::map<CellKey, XCell> xcells_;
			CellKey active_;
			TightRopeCell *prev_, *next_;

	};





};