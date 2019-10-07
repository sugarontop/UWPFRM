#pragma once

#include "D2DUniversalControlBase.h"

namespace V4 {

	class DrawText 
	{
		public :
			DrawText(){}

			void Text(D2DContext& cxt, LPCWSTR str);
			void d(D2DContext& cxt, const FPointF& pt, ID2D1Brush* br);

		private :
			ComPTR<IDWriteTextLayout> t_;
			

	};



	class D2DIsland : public D2DControls
	{
		public :
			D2DIsland();

			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id);

			void ModeChange();
			FRectF* AnimeRect(){ return &rcMin_; }
		protected :
			enum MODE { NORMAL, BIT };
			MODE mode_;
			int WndProcN(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);
			int WndProcB(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);

			FRectF rcMin_;
			DrawText title_;

	};

	struct RectSqueeze
	{
		FRectF* target;
		FRectF frc;			// from rect
		FRectF trc;			// to rect


		RectSqueeze():target(0), prc_(0){}
		~RectSqueeze(){ delete [] prc_; }

		
		FRectF* prc_;
	};

};