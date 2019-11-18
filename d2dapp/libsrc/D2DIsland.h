#pragma once

#include "D2DUniversalControlBase.h"

namespace V4 {

	class DrawText 
	{
		public :
			DrawText(){}

			FSizeF Text(D2DWindow* win, LPCWSTR str);
			FSizeF Text(D2DContext& cxt, LPCWSTR str);
			void d(D2DContext& cxt, const FPointF& pt, ID2D1Brush* br);

		private :
			ComPTR<IDWriteTextLayout> t_;
			

	};

	class D2DIslandTitlebar : public D2DControl
	{
		public :
			D2DIslandTitlebar();
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id);

			void ShowTitleBar(bool bShow);


			virtual void OnDXDeviceLost() override;
			virtual void OnDXDeviceRestored(D2DContext& cxt) override;
		private :
			DrawText title_;
			FRectF rcFilter_;
			ComPTR<ID2D1SolidColorBrush> back_;
			FSizeF szTitle_;
	};



	class D2DIsland : public D2DControls
	{
		public :
			D2DIsland();

			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id);

			void ModeChange();
			FRectF* AnimeRect(int md){ return ( md==0 ? &rc_ : &rcMin_); }
			
			void ShowTitleBar( bool bShow );
		protected :
			enum MODE { NORMAL, MINIBAR };
			MODE mode_;
			int WndProcN(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);
			int WndProcB(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);

			FRectF rcMin_;
			DrawText title_;
			int clridx_;
			D2DIslandTitlebar* titlebar_;
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


	struct FRectF2
	{
		FRectF frc;
		FRectF trc;
		FRectF lastrc;
	};


	class D2DLery : public D2DControls
	{
		public :
			D2DLery(){}
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id);

			static DWORD CALLBACK anime1(LPVOID p);
		protected:
			void Squeeze(bool isvisible);
			FRectF2* target_;
			int idx_;
			
	};

	class D2DLeryRadioButton : public D2DControl
	{
		public :
			D2DLeryRadioButton(int& idx):idx_(idx){};
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id);
		protected:
			bool checked_;
			int& idx_;
	};



	struct FRectF3
	{
		FRectF frc;
		FRectF trc;
		FRectF lastrc;
		LPVOID obj;
		byte md;
	};

	struct MenuItem
	{
		std::wstring viewnm;
		int message_id;

	};


	class D2DSliderButton : public D2DControl
	{
		public :
			D2DSliderButton(int btncnt, bool isModal):btncnt_(btncnt), isModal_(isModal), isVertical_(false){}
			D2DSliderButton();
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id);


			void DrawSqueeze();

			void Set( D2DControl* target, const std::vector<MenuItem>& items);
			void Close();
		protected :
			int WndProcA(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);
			int WndProcB(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);
		protected :
			int btncnt_;
			std::shared_ptr<FRectF[]> btn_;
			FRectF rcFilter_;
			bool isModal_;
			std::vector<MenuItem> items_;
			bool isVertical_;
			int float_idx_;
			D2DControl* target_;
	};

	int RightButtonFloatMenu(FPointF pt, D2DControl* parent, std::vector<MenuItem>& items, ColorF backclr);

};