#pragma once
#include "d2dcontext.h"
#include "d2dcommon.h"
#include "vectorstack.h"
#include "threadlock.h"
#include "D2DWindowMessage.h"

#define  DRGB ColorPtr

namespace V4 {

typedef void (*D_FillRect)( D2DContext& cxt, D2D1_RECT_F& rc );


enum STAT{ VISIBLE=0x1,CAPTURED=0x2,BORDER=0x4,AUTOSIZE=0x8, FOCUS=0x10,MOUSEMOVE=0x20,DEAD=0x800, CAPTURED_LOCK=0X1000 };
enum SCROLLBAR_TYP{ VSCROLLBAR, HSCROLLBAR };

struct D2DContext;
class D2DCaptureObject;
class D2DControl;
class D2DControls;
class D2DDriftDialog;
class D2DScrollbar;
class Caret;

typedef std::vector<std::shared_ptr<D2DControl>> vectorD2DControl;

class D2DWindow
{
	public:				
		virtual D2DContext* cxt()=0;		
		virtual void redraw() = 0;
		virtual void AddDeath( std::shared_ptr<D2DControl> obj ) = 0;
		virtual int SendMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)=0;
		virtual int PostMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)=0;

	public :
		void InvalidateRect(){redraw();}
		
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

		virtual void InnerCreateWindow(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid);

		virtual void UnActivate();
		virtual void OnSetCapture(int layer) override;
		virtual void OnReleaseCapture(int layer) override;
		virtual void UpdateScrollbar(D2DScrollbar* ){}
		virtual void OnDXDeviceLost(){};
		virtual void OnDXDeviceRestored(D2DContext& cxt){};

		virtual void SetText( LPCWSTR txt ){};
		virtual std::wstring GetText() const { return L""; }


		virtual void DestroyControl();
		virtual bool IsCaptured() const;
		virtual void DoCapture();

		virtual void Visible(){ stat_ |= STAT::VISIBLE; }
		virtual void Hide(){ stat_ &= ~STAT::VISIBLE; }
		
		virtual bool IsHide() const{ return ((stat_ & STAT::VISIBLE )== 0 ); }
		virtual bool IsVisible() const{ return ((stat_ & STAT::VISIBLE )!= 0 ); }

		virtual D2DWindow* GetParentWindow(){ return parent_; }

		virtual D2DControls* ParentExchange( D2DControls* newparent );
		
		virtual int GetStat() const{ return stat_; }
		virtual int GetId(){ return id_; }
		virtual FRectFBoxModel GetRect() const { return rc_; }
		virtual void SetRect(const FRectFBoxModel& rc){ rc_ = rc; }
		virtual D2DControls* GetParentControl(){ return parent_control_; }
		virtual void SetCapuredLock(bool lock );


		virtual IDWriteFactory* GetDWFactory(){ return parent_->cxt()->cxtt.wfactory;}
		virtual IDWriteTextFormat* GetTextFormat() { return parent_->cxt()->cxtt.textformat; }

		virtual void SetNewParentControl(D2DControls* nc){ parent_control_ = nc; }

		virtual void SetTarget(IDispatch* p){ disp_ = p; disp_->AddRef(); }
		virtual IDispatch* GetTarget(){ return disp_; }
		virtual void SetBackground(D_FillRect drawfunction){ back_ground_ = drawfunction;}
		virtual std::wstring GetName() const { return name_; }
		virtual bool IsImportantMsg(UINT msg) const;
		virtual D2DMat GetMat() const{ return mat_; }
		virtual void BackColor(D_FillRect f){ back_ground_ = f; }
	protected :
		D2DMat mat_;
		FRectFBoxModel rc_;
		D2DWindow* parent_;
		D2DControls* parent_control_;
		std::wstring name_;
		int id_;
		IDispatch* disp_;
		int stat_;

		D_FillRect back_ground_;

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
		virtual std::shared_ptr<D2DControl> Detach( D2DControl* target);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;

		virtual FRectF GetInnerRect(int idx=0 ){ return rc_.GetContentRect().ZeroRect() ;}		
		virtual void OnDXDeviceLost() override;
		virtual void OnDXDeviceRestored(D2DContext& cxt) override;


		std::function<FRectF(D2DControl*)> wmsize_;

	protected :		
		virtual int DefWndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);
		virtual int DefPaintWndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);

	protected :
		std::vector<std::shared_ptr<D2DControl>> controls_;		
		VectorStack<D2DCaptureObject*> capture_;
};


struct WParameterMouse
{
	FPointF pt;
	FPointF ptprv;
	FPointF move_ptprv;
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
		
		// B Series capture
		virtual void BAddCapture(D2DCaptureObject* cap);	
		virtual D2DCaptureObject* BGetCapture();
		virtual void BReleaseCapture(D2DCaptureObject* target=nullptr);
		virtual bool BCaptureIsEmpty(){ return test_cap_.empty(); }
		virtual VectorStack<D2DCaptureObject*> BCopyCapture(){ return test_cap_; }
		virtual D2CoreTextBridge* GetImeBridge(){ return imebridge_; }

		virtual int SendMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		virtual int PostMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;


		virtual D2DControl* FindControl(LPCWSTR name );
		void ReSize();
		void SetBkColor(ColorF clr){ back_color_=clr;}

		static void SetCursor(int idx);

	protected :
		int PostWndProc( D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp );
		int WndProcOne(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp);

		D2DControls dumy_;
		D2DContext cxt_;
		VectorStack<D2DCaptureObject*> test_cap_;
		bool IsMousemessageDiscard_;

		struct PostMessageStruct
		{
			int message;
			INT_PTR wp;
		};

		std::vector<PostMessageStruct> post_message_ar_;
		std::vector<std::shared_ptr<D2DControl>> pre_death_objects_;

		void TimerSetup();
		void DoDestroy();
		void SampleRender(LPCWSTR msg);

		static Windows::UI::Core::CoreCursor^ cursor_[5];
		WParameterMouse mosue_wp_;
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

struct WParameter
{
	WParameter():sender(0),target(0),prm(0),no(0){}
	D2DControl* sender;
	D2DControl* target;
	void* prm;
	int no;
};

struct WParameterString
{
	BSTR str1;
	BSTR str2;
	int idx;
};
struct WParameterFocus
{
	D2DControl* newfocus;
	D2DCaptureObject* prvfocus;
	FPointF pt;

};

class Script
{
	public :
		Script(){}
		virtual bool ExecBSTR( LPCWSTR function_name, BSTR* ret ){ return 0; }
		
};


struct DllBridge
{
	DllBridge():ctrls(0),caret(0),factory(0),script(0){}
	D2DControls* ctrls;
	Caret* caret;
	void*  factory;
	Script* script;

};

#define LOGPT(xpt,wp) FPointF xpt = mat_.DPtoLP(*(FPointF*)(wp));

#define CXTM(dd) auto& cxt = *(dd->cxt());D2DMatrix mat(cxt);

}; // V4