#pragma once


#define WM_D2D_APP_INTERNET_GET		(WM_D2D_USERCD+1)


namespace V4 {
namespace V4_XAPP1 {
class D2DButton : public D2DControl
{
	public :
		D2DButton(){}

		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR title, LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)  override;
	
		virtual void SetText( LPCWSTR txt ){title_= txt;}
		virtual std::wstring GetText() const { return title_; }


		static void DefaultDrawButton( D2DButton* sender, D2DContext& cxt );
	protected :
		int mode_;
		std::wstring title_;
		ComPTR<ID2D1LinearGradientBrush> br_;
	public :
		std::function<void(D2DButton* sender)> OnClick_;
		std::function<void(D2DButton* sender, D2DContext& cxt)> OnPaint_;


};







};
};