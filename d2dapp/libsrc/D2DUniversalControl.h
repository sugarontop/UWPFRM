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
		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);

		void ParseMenu(BSTR json);
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
		D2DControl* target_;

		void DrawItem( D2DContext& cxt, Item& it );
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

		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR title, LPCWSTR name, int local_id = -1);
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


class D2DTabControls: public D2DControls
{
	public :
		D2DTabControls();
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
			
		void Update(int idx);
		FRectF GetContentRect() const;
	protected:
		void DrawTab(D2DContext& cxt);
		void TitleReset();

		struct Tab
		{
			FRectF rc;
			ComPTR<IDWriteTextLayout> text;
			std::shared_ptr<D2DControl> control;
			FSizeF textsize;
			std::wstring title;

		};


		std::vector<Tab> tabs_;
		UINT active_idx_;
		std::shared_ptr<D2DControl> active_;


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

		D2DMessageBox(){}
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid);

	protected :
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
		virtual void OnDXDeviceRestored(D2DContext& cxt) override;

		
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
		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);

		// Textbox‚ÅŽg—p
		void CreateWindowEx(D2DWindow* parent, D2DControl* target, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		float OffsetOnBtn( int typ );
		void SetTotalSize( float size );

		D2DScrollbarInfo& Info(){ return info_; }
		
		const D2D1_RECT_F& GetContentRect();

		void OtherHand(bool bl);
		void SetRowHeight( float rowheight );
		void SetScrollbarTop();

		virtual void OnDXDeviceLost() override;
		virtual void OnDXDeviceRestored(D2DContext& cxt) override;
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
		
		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;

};

class D2DChildFrame2 :public D2DControls
{
	public :
		D2DChildFrame2(){};
		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc,int stat,LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void SetCanvasSize( float cx, float cy );
	protected:
		virtual void UpdateScrollbar(D2DScrollbar* bar) override;

		void DrawDefault(D2DContext& cxt, D2DWindow* d, INT_PTR wp);
		float DrawTitle( D2DContext& cxt, const FRectF& rc );
		

		FRectFBoxModel VScrollbarRect( const FRectFBoxModel& rc );
		FRectFBoxModel HScrollbarRect( const FRectFBoxModel& rc );
		int InnerDefWndScrollbarProc(D2DWindow* d, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam);
		bool TitlebarDblclick();
		void Resize();
	private :
		enum MODE { NONE, MOVING,RESIZE,SCROLLBAR };
		std::shared_ptr<D2DControl> Vscbar_;
		std::shared_ptr<D2DControl> Hscbar_;
		ComPTR<IDWriteTextLayout> title_;
		bool titlebar_enable_;
		float scale_;	
		FSizeF scrollbar_off_;
		MODE md_;
		FRectF prrc_;

		bool active_;
		
	public :
		std::map<int, std::function<int(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)>> Extention_;
};

class D2DTransparentControls : public D2DControls
{
	public :
		D2DTransparentControls(){}
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void Create(D2DControls* pacontro, LPCWSTR name, int id);
};


class InnerListbox : public D2DControl
{
	public :
		InnerListbox();
		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc,int stat,LPCWSTR name, int local_id = -1, int selidx=-1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;

		void Close(bool bDestroy=true);

		std::vector<IDWriteTextLayout*> ar_;
		ComPTR<ID2D1SolidColorBrush> floadbr_;
		int float_idx_;
		int md_;
};

class D2DDropDownListbox : public D2DControls
{
	public :
		D2DDropDownListbox();
		virtual ~D2DDropDownListbox(){ Clear(); }
		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc,int stat,LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;

		void AddItem( LPCWSTR key, LPCWSTR value );
		int GetSelectIndex() const { return selected_idx_; }
		void Clear();
		std::wstring Value(int idx);
		void SetSelectIndex( int idx );
	public :

	private :
		struct Item
		{
			int idx;
			std::wstring key;
			ComPTR<IDWriteTextLayout> layout;
		};

		std::vector<Item> ar_;
		std::map<std::wstring,std::wstring> items_;

		int selected_idx_;
};


class IListboxItem
{
	public :
		IListboxItem():bSelected_(false){}
		virtual ~IListboxItem(){}
		virtual FRectF GetRect() = 0;
		virtual void Draw(D2DContext& cxt) = 0;
		void SetSelect(bool bl){ bSelected_=bl; }
		bool IsSelect(){ return bSelected_; }
	protected :
		bool bSelected_;
};

class ListboxItemString : public IListboxItem
{
	public :
		ListboxItemString(){};
		ListboxItemString(D2DContext& cxt, LPCWSTR txt);
		virtual ~ListboxItemString(){ Clear(); }
		virtual FRectF GetRect() override;
		virtual void Draw(D2DContext& cxt) override;
	public :
		void Clear();
		void SetText(ComPTR<IDWriteTextLayout> layout);		
	private :
		ComPTR<IDWriteTextLayout> layout_;
		
};

class D2DListbox : public D2DControls
{
	public :
		enum TYP{ SINGLELINE, MULTILINE };
		
		D2DListbox(){}
		virtual ~D2DListbox(){ Clear(); }
		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc,int stat,LPCWSTR name, TYP typ, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;

		virtual void AddItem(std::shared_ptr<IListboxItem> item);
		virtual void AddItem(LPCWSTR str);

		void Clear();

		void SetParam(LPCWSTR json);

	public :
		virtual void UpdateScrollbar(D2DScrollbar* ) override;
		void SetSelectIndex( int idx );
		void ClearSelect();
	private :
		std::vector<std::shared_ptr<IListboxItem>> ar_;
		TYP typ_;
		int float_idx_;
		float items_height_;
		int md_;
		int start_view_row_;

		D2DScrollbar* vbar_;
		FSizeF scrollbar_off_;

		D2D1_COLOR_F clr_float_;
		D2D1_COLOR_F clr_select_;

};


FSizeF CreateTextLayout( D2DContext& cxt, LPCWSTR str, UINT strlen, IDWriteTextLayout** ret );
ComPTR<ID2D1SolidColorBrush> CreateBrush( D2DContext& cxt, D2D1_COLOR_F clr );
bool CreateLightThread(LPTHREAD_START_ROUTINE th, LPVOID prm, DWORD* thread_id);
DWORD CreateLightThread(LPTHREAD_START_ROUTINE th, LPVOID prm);



};
