#pragma once
#include "d2dcontext.h"
#include "d2dcommon.h"
#include "vectorstack.h"
#include "threadlock.h"

#define  DRGB ColorPtr




namespace V4 {

enum STAT{ VISIBLE=0x1,CAPTURED=0x2,BORDERLESS=0x4,AUTOSIZE=0x8, DEAD=0x800, CAPTURED_LOCK=0X1000 }; //MOUSEMOVE=0x2,SELECTED=0x10
enum SCROLLBAR_TYP{ VSCROLLBAR, HSCROLLBAR };

class D2DCaptureObject;
class D2DControl;
class D2DControls;
class D2DDriftDialog;
class D2DScrollbar;


typedef std::vector<std::shared_ptr<D2DControl>> vectorD2DControl;

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
		D2DControl();
		virtual ~D2DControl();

		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);

		void InnerCreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid);

		virtual void UnActivate();
		virtual void OnSetCapture(int layer) override;
		virtual void OnReleaseCapture(int layer) override;
		virtual void UpdateScrollbar(D2DScrollbar* ){}
		virtual void OnDXDeviceLost(){};
		virtual void OnDXDeviceRestored(){};

		virtual void SetText( LPCWSTR txt ){};
		virtual std::wstring GetText() const { return L""; }


		virtual void DestroyControl();
		bool IsCaptured() const;
		void DoCapture();

		void Visible(){ stat_ |= STAT::VISIBLE; }
		void Hide(){ stat_ &= ~STAT::VISIBLE; }
		
		bool IsHide() const{ return ((stat_ & STAT::VISIBLE )== 0 ); }
		bool IsVisible() const{ return ((stat_ & STAT::VISIBLE )!= 0 ); }

		D2DWindow* GetParentWindow(){ return parent_; }

		D2DControls* ParentExchange( D2DControls* newparent );
		
		int GetStat() const{ return stat_; }
		int GetId(){ return id_; }
		FRectFBoxModel GetRect() const { return rc_; }
		void SetRect(const FRectFBoxModel& rc){ rc_ = rc; }
		D2DControls* GetParentControl(){ return parent_control_; }
		void SetCapuredLock(bool lock );


		IDWriteFactory* GetDWFactory(){ return parent_->cxt()->cxtt.wfactory;}
		IDWriteTextFormat* GetTextFormat() { return parent_->cxt()->cxtt.textformat; }

		void SetNewParentControl(D2DControls* nc){ parent_control_ = nc; }

		void SetTarget(IDispatch* p){ disp_ = p; disp_->AddRef(); }
		IDispatch* GetTarget(){ return disp_; }
	protected :
		D2DMat mat_;
		FRectFBoxModel rc_;
		D2DWindow* parent_;
		D2DControls* parent_control_;
		std::wstring name_;
		int id_;
		IDispatch* disp_;
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
		

		virtual D2DCaptureObject* ReleaseCapture(D2DCaptureObject* target=nullptr, int layer=-1);		
		virtual D2DCaptureObject* GetCapture();		
		std::shared_ptr<D2DControl> Detach( D2DControl* target);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;

		virtual FRectF GetInnerRect(int idx=0 ){ return rc_.GetContentRect().ZeroRect() ;}		
		virtual void OnDXDeviceLost() override;
		virtual void OnDXDeviceRestored() override;
	protected :		
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
		virtual void Close() = 0;

		// D2DControls
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)  override;

		

		void AliveMeter(Windows::System::Threading::ThreadPoolTimer^ timer);
		FRectF GetMainWndRect(){ return rc_; }
		
		// Bseries capture
		void BAddCapture(D2DCaptureObject* cap);	
		D2DCaptureObject* BGetCapture();
		void BReleaseCapture(D2DCaptureObject* target=nullptr);
		bool BCaptureIsEmpty(){ return test_cap_.empty(); }
		VectorStack<D2DCaptureObject*> BCopyCapture(){ return test_cap_; }


		virtual int SendMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		virtual int PostMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;

		static void SetCursor(int idx);

	protected :
		int PostWndProc( int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp );

		D2DControls dumy_;
		D2DContext cxt_;
		VectorStack<D2DCaptureObject*> test_cap_;

		struct PostMessageStruct
		{
			int message;
			INT_PTR wp;
		};

		std::vector<PostMessageStruct> post_message_ar_;
		std::vector<std::shared_ptr<D2DControl>> pre_death_objects_;

		void TimerSetup();
		void DoDestroy();

		static Windows::UI::Core::CoreCursor^ cursor_[5];

	public  :
		bool redraw_;
		ColorF back_color_;
		D2CoreTextBridge* imebridge_;
		std::map<std::wstring,D2DControl*> hub_;
		DWORD gui_thread_id_;
		thread_gui_lock lock_;

		D2DCaptureObject* cap_;
	public :
		typedef std::function<void(int, bool*)> timerfunc;
		std::vector<timerfunc> timerfuncs_;
};



}; // V4