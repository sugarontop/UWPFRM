#pragma once

#include "D2DUniversalControlBase.h"

namespace V4 {

class FontInfo;

class D2DTitlebarMenu : public D2DControls
{
	public :
		D2DTitlebarMenu(){}
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		virtual int HideMenu();

		virtual void OnReleaseCapture(int layer) override;
		virtual void OnSetCapture(int layer) override;

		std::vector<FRectF> items_;
		int floating_idx_;
		int selected_idx_;
};

class D2DVerticalMenu : public D2DControl
{
	public :
		D2DVerticalMenu(){}
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);


	private :
		struct Item
		{
			int id;
			std::wstring name;
			int menuid;
			
			FRectF rc;

		};
		std::vector<Item> items_;
		int float_pos_;

		void Draw( D2DContext& cxt, Item& it );
};



class D2DGroupControls : public D2DControls
{	
	public :
		enum TYP { HW_FIXSIZE, HEIGHT_FLEXIBLE, WIDTH_FLEXIBLE };
		enum WNDTYP { NONE, LEFTSIDE };

		D2DGroupControls():ty_(HW_FIXSIZE),back_(D2RGB(192,192,192)){}
		D2DGroupControls(TYP ty):ty_(ty),back_(D2RGB(192,192,192)){}

	public :
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, WNDTYP wd=WNDTYP::NONE,int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void SetBackColor( ColorF back ){ back_ = back; }
		

		void SetDriftControl( int typ,float driftvalue, D2DControls* ctrls );

		D2DControls* GetInner(int idx);

	protected :
		D2DDriftDialog* drift_;
		D2DTitlebarMenu* menu_;
		D2DControls* center_;
		TYP ty_;
		ColorF back_;
		int drift_typ_;
		float drift_value_;
};



class D2DButton : public D2DControl
{
	public :
		D2DButton(){}

		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR title, LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);
	
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







class D2DStatic : public D2DControl
{
	public :
		D2DStatic();
		~D2DStatic();
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void SetText( LPCWSTR txt, int align, IDWriteTextFormat* tf=nullptr );
		void SetFont( const FontInfo& cf, int typ=-1 );
		void SetForeColor( ColorF fore ){ fore_ = fore; }
		
	protected :
		void Clear();
		
	protected :
		ColorF fore_;
		ComPTR<IDWriteTextLayout> layout_;
		FRectF _rc() const;
		BSTR text_;
};





class D2DMessageBox : public D2DControls
{
	public :		
		static void Show(D2DWindow* parent, const FRectF& rc, LPCWSTR title, LPCWSTR msg, int typ=0 );

	protected :
		D2DMessageBox(){}
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid);

		ComPTR<IDWriteTextLayout> title_;
		ComPTR<IDWriteTextLayout> msg_;


		int result_;
		int typ_;

};

// DropdownListbox
class D2DCombobox : public D2DControl
{
	public :
		D2DCombobox();
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		int AddItem( LPCWSTR key, LPCWSTR value );
		void Clear();
		int SelectedIdx() const { return select_idx_; }
		std::wstring SelectedKey() const { return items_[select_idx_].key; }
		std::wstring SelectedValue() const { return  items_[select_idx_].value; }

		virtual void OnReleaseCapture(int layer) override;

		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;

		void SetSelectedIdx( int idx );

		struct wparam
		{
			D2DCombobox* cmb;

		};

		std::function<void(D2DCombobox*)> OnSelectChanged_;
	protected :
		void DrawSelectedText(D2DWindow* d);
		void DrawListbox(D2DWindow* d);
		void OnSelectChanged();
	protected :
		struct KeyValue
		{
			std::wstring key,value;
		};

		int select_idx_;
		int floating_relative_idx_;
		std::vector<KeyValue> items_;
		float ListboxHeight_;
		int md_;
		UINT start_idx_;

		

};


class D2DControlsWithScrollbar : public D2DControls
{
	public :
		D2DControlsWithScrollbar(){}
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam) override;
		void Create( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, bool bAutoSize,  int id=-1  );
		
		virtual void UpdateScrollbar(D2DScrollbar* bar);
		void SetTotalSize( float cx, float cy );
		void ShowScrollbar( SCROLLBAR_TYP typ, bool visible );

		virtual void OnDXDeviceLost() override;
		virtual void OnDXDeviceRestored() override;

		
	private :
		int DefWndScrollbarProc(D2DWindow* parent, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam);

		std::shared_ptr<D2DControl> Vscbar_;
		std::shared_ptr<D2DControl> Hscbar_;

		FSizeF scrollbar_off_;
		float scale_;		
};

class D2DScrollbar : public D2DControl
{
	public :
		D2DScrollbar();
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam) override;
		virtual void OnCreate();
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);

		// Textbox‚ÅŽg—p
		void CreateWindowEx(D2DWindow* parent, D2DControl* target, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		float OffsetOnBtn( int typ );
		void SetTotalSize( float size );

		D2DScrollbarInfo& Info(){ return info_; }
		
		const D2D1_RECT_F& GetContentRect();

		void OtherHand(bool bl);
		void SetRowHeight( float rowheight );

		virtual void OnDXDeviceLost() override;
		virtual void OnDXDeviceRestored() override;
	protected :
		void SetScrollBarColor( D2DContext& cxt );
	protected :
		bool OtherHand_;
		D2DScrollbarInfo info_;
		D2DControl* target_control_;		
		
};


class D2DDebugPlaceHolder : public D2DControls
{	
	public :		
		D2DDebugPlaceHolder();
		D2DDebugPlaceHolder(ColorF bk);
		~D2DDebugPlaceHolder();
	public :
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1) ;
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		
		static void DrawDebugInfo( D2DDebugPlaceHolder* sender, D2DContext& cxt );
		FPointF mouse_pt_;
		ComPTR<IDWriteTextLayout> layout1_;
		ColorF backclr_;
};

struct SlideMenuItem
{
	D2DControls* sender;
	D2DControls* item;
	std::function<void(void)> OnComplete;

};


class D2DChildControls : public D2DControls
{
	public :
		D2DChildControls(){};

		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat,LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		//virtual void OnReleaseCapture(int layer) override;
		//virtual void OnSetCapture(int layer) override;

		std::map<int, std::function<int(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)>> Extention_;

	private :
		enum MODE { NONE, MOVE };
		MODE mode_;

};

class D2DChildFrame :public D2DControls
{
	public :
		D2DChildFrame(){};
		
		enum WINSTYLE { DEFAULT=0 };

		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat,WINSTYLE ws, LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		virtual void OnReleaseCapture(int layer) override;
		virtual void OnSetCapture(int layer) override;

		void SetScale( float scale );

		static FRectFBoxModel VScrollbarRect( const FRectFBoxModel& rc );
		static FRectFBoxModel HScrollbarRect( const FRectFBoxModel& rc );

	protected :
		void DrawTitle( D2DContext& cxt, const FRectF& rc );
		void DrawDriftRect(D2DWindow* d, D2DContext& cxt);
		
		enum WINDOWMODE { NORMAL, MAXMIZE, MINIMIZE };
		enum MODE { NONE, MOVING,RESIZE };

		enum FMODE { TRY, DO };
		bool TB_MouseWindowResize( FMODE mode, FRectF rc, FPointF pt );
		bool TB_DlgWindowProperty( FMODE mode, FRectF rc, FPointF pt );
		bool TB_WindowClose( FMODE mode, FRectF rc, FPointF pt );
		bool TB_MinimizeWindow( FMODE mode, FRectF rc, FPointF pt );
		bool TB_MDIDetach( FMODE mode, FRectF rc, FPointF pt );

		void TitlebarDblclick();
		void MDI_Docking( bool IsDocking, D2DChildFrame* k );
		void MDI_Docking( bool IsDocking, D2DMainWindow* k );

		

	//scrollbar///////////////////
	public :
		virtual void UpdateScrollbar(D2DScrollbar* bar) override;
		void SetCanvasSize( float cx, float cy );
		void ShowScrollbar( SCROLLBAR_TYP typ, bool visible );
		
	private :
		int InnerDefWndScrollbarProc(D2DWindow* parent, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam);

		void DrawDefault(D2DContext& cxt, D2DWindow* d, INT_PTR wp);
		void DrawMinimize(D2DContext& cxt, D2DWindow* d, INT_PTR wp);

	private :
		std::shared_ptr<D2DControl> Vscbar_;
		std::shared_ptr<D2DControl> Hscbar_;

		FSizeF scrollbar_off_;
		
		byte* test_;


		struct MDI_Prev
		{
			vectorD2DControl kls;
			vectorD2DControl hls;
			D2DChildFrame* h;
		};

		MDI_Prev mdi_prev_;
		
		WINDOWMODE wmd_;
		FRectFBoxModel prv_rc_;
		int md_;
		ComPTR<IDWriteTextLayout> title_;
		float scale_;		

		struct InfoDrift
		{
			HANDLE cc;
			FRectF dstRect;
			std::function<void()> completed;
		};

		std::shared_ptr<InfoDrift> drift_;

		FPointF ptold;

	public :
		std::map<int, std::function<int(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)>> Extention_;
};

};
