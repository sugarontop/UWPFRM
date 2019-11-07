#pragma once

namespace V4 {
class D2DControlsView : public D2DControls
{	
	public :
		D2DControlsView():back_(D2RGB(192,192,192)){}
		
		void Create( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id );
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;

		void SetBack(ColorF clr){ back_ = clr; }
	protected :
		ColorF back_;
};

class D2DSplitControls : public D2DControls
{	
	public :
		enum TYP { NONE,LEFT_RIGHT,LEFT_RIGHT_SLIDE};

		D2DSplitControls():ty_(LEFT_RIGHT),back_(D2RGB(192,192,192)){}
		
		void Create( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id );
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		virtual FRectF GetInnerRect(int idx=0 ) override;


		D2DControls* GetInner(int idx);

		void SetInner( D2DControls* newcontrols, bool bLeft );


	protected :
		TYP ty_;
		ColorF back_;

		D2DControls* left_;
		D2DControls* right_;

};


class D2DSlideControls : public D2DSplitControls
{	
	public :
		D2DSlideControls();
		void Create( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id );
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;

		virtual FRectF GetInnerRect(int idx ) override;
	protected :
		void OnDrawDefault(D2DContext& cxt,INT_PTR wp, float xoff );

		bool bSliding_;
		void Sliding(  bool bRight2Left );
		HANDLE cc_;
		int md_;
};



};
