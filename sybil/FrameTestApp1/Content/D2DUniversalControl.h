#pragma once

#include "d2dcontext.h"
#include "d2dcommon.h"
#include "vectorstack.h"
#include "threadlock.h"

#define  DRGB ColorPtr

namespace V4 {

enum STAT{ VISIBLE=0x1,CAPTURED=0x2,BORDERLESS=0x4,AUTOSIZE=0x8, DEAD=0x800 }; //MOUSEMOVE=0x2,SELECTED=0x10
enum SCROLLBAR_TYP{ VSCROLLBAR, HSCROLLBAR };

class D2DCaptureObject;
class D2DControl;
class D2DControls;
class D2DDriftDialog;
class D2DScrollbar;

class D2DWindow
{
	public:				
		virtual D2DContext* cxt()=0;		
		virtual void redraw() = 0;
		virtual void AddDeath( std::shared_ptr<D2DControl> obj ) = 0;
		virtual int SendMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)=0;
		virtual int PostMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)=0;
		
};

class D2DCaptureObject
{
public:
	virtual ~D2DCaptureObject(){}
	virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)=0;
	virtual void OnReleaseCapture(int layer)=0;
	virtual void OnSetCapture(int layer)=0;
	
};



class D2DControl : public D2DCaptureObject
{
	friend class D2DControls;
	public :
		virtual ~D2DControl(){}

		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);

		void InnerCreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid);

		virtual void UnActivate();
		virtual void OnSetCapture(int layer) override;
		virtual void OnReleaseCapture(int layer) override;
		virtual void UpdateScrollbar(D2DScrollbar* ){}

		virtual void DestroyControl();
		bool IsCaptured() const;
		void DoCapture();

		void Visible(){ stat_ |= STAT::VISIBLE; }
		void Hide(){ stat_ &= ~STAT::VISIBLE; }
		int GetStat() const{ return stat_; }
		bool IsHide() const{ return ((stat_ & STAT::VISIBLE )== 0 ); }

		D2DWindow* ParentWindow(){ return parent_; }

		D2DControls* ParentExchange( D2DControls* newparent );


		FRectFBoxModel GetRect() const { return rc_; }
		void SetRect(const FRectFBoxModel& rc){ rc_ = rc; }
		D2DControls* ParentControl(){ return parent_control_; }

		IDWriteFactory* GetDWFactory(){ return parent_->cxt()->cxtt.wfactory;}
		IDWriteTextFormat* GetTextFormat() { return parent_->cxt()->cxtt.textformat; }

	protected :
		D2DMat mat_;
		FRectFBoxModel rc_;
		D2DWindow* parent_;
		D2DControls* parent_control_;
		std::wstring name_;
		int id_;
		void* target_;
		int stat_;
};

class D2DControls : public D2DControl
{
	friend class D2DControl;
	friend class D2DMainWindow;

	protected :
		D2DControls(){}
		

	public :
		virtual void SetCapture(D2DCaptureObject* p, int layer=0 );
		virtual D2DCaptureObject* ReleaseCapture(int layer=-1);		
		virtual D2DCaptureObject* GetCapture();		
		std::shared_ptr<D2DControl> Detach( D2DControl* target);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
	protected :
		D2DCaptureObject* ReleaseCaptureEx(int layer);	

		int DefWndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);
		int DefPaintWndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);
	protected :
		std::vector<std::shared_ptr<D2DControl>> controls_;		
		VectorStack<D2DCaptureObject*> capture_;
};


class D2DMainWindow : public D2DWindow, public D2DControls
{
	public :
		D2DMainWindow();

		virtual D2DContext* cxt()=0;
		virtual void redraw() override { redraw_ = true; }
		virtual void AddDeath( std::shared_ptr<D2DControl> obj )  override;
	

		// D2DControls
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)  override;

		virtual void OnDXDeviceLost(){};
		virtual void OnDXDeviceRestored(){};

		void AliveMeter(Windows::System::Threading::ThreadPoolTimer^ timer);
		FRectF GetMainWndRect(){ return rc_; }
		D2DCaptureObject* SetTopCapture(D2DCaptureObject* cap);
		D2DCaptureObject* GetTopCapture(){ return cap_;}

		virtual int SendMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		virtual int PostMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
	protected :
		int PostWndProc( int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp );

		D2DControls dumy_;
		D2DContext cxt_;

		struct PostMessageStruct
		{
			int message;
			INT_PTR wp;
		};

		std::vector<PostMessageStruct> post_message_ar_;
		std::vector<std::shared_ptr<D2DControl>> pre_death_objects_;

	public :
		bool redraw_;
		ColorF back_color_;
		D2CoreTextBridge* imebridge_;
		std::map<std::wstring,D2DControl*> hub_;
		DWORD gui_thread_id_;
		thread_gui_lock lock_;

		D2DCaptureObject* cap_;

};

class D2DTitlebarMenu : public D2DControls
{
	public :
		D2DTitlebarMenu(){}
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		virtual int HideMenu();

		std::vector<FRectF> items_;
		int floating_idx_;
};

class D2DVerticalMenu : public D2DControl
{
	public :
		D2DVerticalMenu(){}
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
};



class D2DGroupControls : public D2DControls
{	
	public :
		enum TYP { HW_FIXSIZE, HEIGHT_FLEXIBLE, WIDTH_FLEXIBLE };

		D2DGroupControls():ty_(HW_FIXSIZE),back_(D2RGB(192,192,192)){}
		D2DGroupControls(TYP ty):ty_(ty),back_(D2RGB(192,192,192)){}

	public :
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void SetBackColor( ColorF back ){ back_ = back; }
		

		void SetDriftControl( int typ,float driftvalue, D2DControls* ctrls );



	protected :
		D2DDriftDialog* drift_;
		D2DTitlebarMenu* menu_;
		TYP ty_;
		ColorF back_;
		int drift_typ_;
		float drift_value_;
};



class D2DButton : public D2DControl
{
	public :
		D2DButton(){}

		void CreateButton(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR title, LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);

		static void DefaultDrawButton( D2DButton* sender, D2DContext& cxt );
	protected :
		int mode_;
		std::wstring title_;
		ComPTR<ID2D1LinearGradientBrush> br_;
	public :
		std::function<void(D2DButton* sender)> OnClick_;
		std::function<void(D2DButton* sender, D2DContext& cxt)> OnPaint_;


};

class Caret
{
	private :
		Caret();

	public :
		static Caret& GetCaret();

		void Activate( TextInfo& ti );
		void UnActivate();

		void ShowCaret();
		void HideCaret();
		
		bool Draw(D2DContext& cxt );
		FRectF GetCaretRect();

		int bShow_;

		int is_start_change_;
	private :
		TextInfo* ti_;
		Windows::System::Threading::ThreadPoolTimer^ timer_;


};


class FontInfo
{
	public :
		FontInfo();

		ComPTR<IDWriteTextFormat> CreateFormat( IDWriteFactory* wfac ) const;
		
	public :
		float height;
		std::wstring fontname;
		int weight;

};




class D2DTextbox : public D2DControl
{
	
	public :
		enum TYP { SINGLELINE,MULTILINE,PASSWORD };
		D2DTextbox(D2CoreTextBridge& bridge);
		D2DTextbox(D2CoreTextBridge& bridge, TYP ty);

		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		virtual void OnReleaseCapture(int layer) override;

				
		void SetText( LPCWSTR txt );
		
		void SetFont( const FontInfo& cf, int typ=-1 );
		void SetBackColor( ColorF back ){ back_ = back; }
		void SetForeColor( ColorF fore ){ fore_ = fore; }
		void SetAlign( int typ );
		void SetReadonly( bool IsReadOnly );
		

		static void s_SetAlign( IDWriteTextFormat* fmt, int typ );
	protected :
		void Activate( int init_pos);
		
		void DrawSelectArea(D2DContext& cxt);
		void OnTextUpdated();
		
	protected :
		TYP typ_;
		D2CoreTextBridge& bridge_;
		
		TextInfo ti_;
		int shift_control_key_;
		ColorF back_,fore_;
		bool IsReadOnly_;
		
		static bool bMouseSelectMode_;
		ComPTR<IDWriteTextLayout> layout_;
				
		FRectF _rc() const;
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





class D2DMessageBox : public D2DControl
{
	public :
		
		static void Show(D2DWindow* parent,D2DControls* pacontrol, FRectF rc, LPCWSTR title, LPCWSTR msg );
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid);

		std::wstring msg_, title_;
		int result_;

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

		// Textbox�Ŏg�p
		void CreateWindowEx(D2DWindow* parent, D2DControl* target, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		float OffsetOnBtn( int typ );
		void SetTotalSize( float size );

		const D2DScrollbarInfo& Info(){ return info_; }
		
		const D2D1_RECT_F& GetContentRect();

		void OtherHand(bool bl);

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
	protected :
		void DrawTitle( D2DContext& cxt, const FRectF& rc );
		void DrawDriftRect(D2DWindow* d, D2DContext& cxt);
		
		enum WINDOWMODE { NORMAL, MAXMIZE, MINIMIZE };
		enum MODE { NONE, MOVING };

		enum FMODE { TRY, DO };
		bool TB_MouseWindowResize( FMODE mode, FRectF rc, FPointF pt );
		bool TB_DlgWindowProperty( FMODE mode, FRectF rc, FPointF pt );
		bool TB_WindowClose( FMODE mode, FRectF rc, FPointF pt );


		WINDOWMODE wmd_;
		FRectF prv_rc_;
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


	//scrollbar///////////////////
	public :
		virtual void UpdateScrollbar(D2DScrollbar* bar);
		void SetCanvasSize( float cx, float cy );
		void ShowScrollbar( SCROLLBAR_TYP typ, bool visible );
		
	private :
		int DefWndScrollbarProc(D2DWindow* parent, int message, INT_PTR wParam, Windows::UI::Core::ICoreWindowEventArgs^ lParam);

		std::shared_ptr<D2DControl> Vscbar_;
		std::shared_ptr<D2DControl> Hscbar_;

		FSizeF scrollbar_off_;
		
};

};