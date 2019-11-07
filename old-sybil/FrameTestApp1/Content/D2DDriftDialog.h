#pragma once
namespace V4 {


class D2DDriftDialog : public D2DControls
{	
	public :		
		D2DDriftDialog();
	public :
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);
		void SetDriftRect( const FRectF& rc );

		void SetRect( const FRectF& rc )
		{
			rc_ = rc;
			orgRect_ = rc;
		}

		std::function<void(D2DDriftDialog*, D2DContext&cxt)> default_v_;

		std::function<void(D2DDriftDialog*)> OnCreate_;

		enum MODE { MIN, MIN2MAX, MAX, MAX2MIN };

		bool bfresize_;
		FRectF dstRect_, orgRect_;

		MODE md_;
		HANDLE cc_;
		
	protected :
		int mode_;

		void OnClosed();
	
};
























};