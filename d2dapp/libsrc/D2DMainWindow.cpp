#include "pch.h"
#include "D2DUniversalControlBase.h"
#include "D2DTextbox.h"


using namespace Windows::System::Threading;
using namespace Windows::UI::Core;

using namespace V4;

class CToolTipImplement : public CToolTip
{
public:
	CToolTipImplement():tick_(0){};
	virtual bool Show(D2DMainWindow* d, LPCWSTR str, FPointF pt);
	virtual bool Hide();
	virtual void Draw(D2DMainWindow* d);
public:
	ComPTR<IDWriteTextLayout> layout_;
	FRectFBoxModel rc_;
	ULONGLONG tick_;

};


Windows::UI::Core::CoreCursor^ D2DMainWindow::cursor_[5];

D2DMainWindow::D2DMainWindow():back_color_(D2RGB(195,195,195))
{
	parent_control_ = &dumy_;
	
	tooltip_ = std::shared_ptr< CToolTipImplement>(new CToolTipImplement());
	imebridge_ = nullptr;
	gui_thread_id_ = ::GetCurrentThreadId();
	lock_.Init();
	gui_lock_ = false;
	focus_ = nullptr;


	cursor_[CURSOR_ARROW] = ref new CoreCursor(CoreCursorType::Arrow,0);
	cursor_[CURSOR_IBeam] = ref new CoreCursor(CoreCursorType::IBeam,0);
	cursor_[CURSOR_Hand] = ref new CoreCursor(CoreCursorType::Hand,0);
	cursor_[CURSOR_SizeWestEast] = ref new CoreCursor(CoreCursorType::SizeWestEast,0);
	cursor_[CURSOR_SizeNorthwestSoutheast] = ref new CoreCursor(CoreCursorType::SizeNorthwestSoutheast,0);

	SetCursor(CURSOR_ARROW);

}

int D2DMainWindow::PostWndProc(D2DWindow* parent, int, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp )
{
	int ret = 0;	
	{
		bool bpaint = false;
		for( auto& itm : post_message_ar_ )
		{
			if ( itm.message == WM_PAINT )
				bpaint = true;
			else
			{			
				for( auto& it : controls_ )
				{
					if ( 0 != ( ret= it->WndProc(this, itm.message, itm.wp, nullptr)) )
					{
						bpaint = true;
						break;
					}
				}			
			}
		}
		post_message_ar_.clear();

		// WM_PAINTは１回だけにまとめた
		if (bpaint)
			WndProcOne(parent, WM_PAINT, 0, nullptr);
	}
	return ret;
}
int D2DMainWindow::WndProc(D2DWindow* parent, int msg, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{		
	if (!post_message_ar_.empty())
	{
		thread_scope sc(lock_);		
		{			
			 int ret = PostWndProc( parent, -1, wp, lp ); // Queue内のmessageを優先処理する

			 if ( ret )
				return ret;
		}
	}


	return WndProcOne(parent,msg,wp,lp);
}

int D2DMainWindow::PostMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	// ここは別スレッドから呼び出されるかもしれないので、スレッドロックをかける。

	thread_scope sc(lock_);

	PostMessageStruct st;
	st.message = message;
	st.wp = wp;

	post_message_ar_.push_back(st);
	
	return 0;
}

int D2DMainWindow::WndProcOne(D2DWindow * parent, int msg, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs ^ lp)
{
	int ret = 0;
	switch( msg )
	{		
		case WM_PAINT:
		{
			DWORD dt = ::GetTickCount();

			ID2D1DeviceContext* cxt = (ID2D1DeviceContext*)cxt_.cxt.p;  // see. void D2DContext::Init
			
			cxt_.tickcount_ = ::GetTickCount(); // milli seconds.
			redraw_ = false;
			
			cxt->SaveDrawingState( cxt_.m_stateBlock);
			cxt->BeginDraw();
			D2D1_MATRIX_3X2_F mat = Matrix3x2F::Identity();

			
			cxt->SetTransform(mat);
			cxt->Clear(back_color_);

			DefPaintWndProc( this, WM_PAINT,0,nullptr );
	
			
			
			tooltip_->Draw(this);



			redraw_ = (redraw_ || Caret::GetCaret().Draw( cxt_ ));

			//D2DERR_RECREATE_TARGET をここで無視します。このエラーは、デバイスが失われたことを示します。
			// これは、Present に対する次回の呼び出し中に処理されます。
			HRESULT hr = cxt->EndDraw();
			if (hr != D2DERR_RECREATE_TARGET)
			{
				//DX::ThrowIfFailed(hr);
			}

			cxt->RestoreDrawingState(cxt_.m_stateBlock);

			
			//TRACE( L"PAINT=%d\n", ::GetTickCount()-dt );
			

			return 0;
		}
		break;		
		case WM_MOUSEMOVE:
		{			
			mosue_wp_.pt = FPointF(lp);

			ret = D2DControls::DefWndProc(this,msg,(INT_PTR)&mosue_wp_,lp);

			mosue_wp_.move_ptprv = FPointF(lp);
		}
		break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			if (gui_lock_) return 1;

			auto s1 = GetCapture();
				


			while(1)
			{
				mosue_wp_.pt = mosue_wp_.move_ptprv = FPointF(lp);
				IsMousemessageDiscard_ = false;
			
				int r1 = SendMessage(WM_D2D_MOUSEACTIVATE, (INT_PTR)&mosue_wp_,nullptr);

				bool IsActivateWindow = false;

				switch (r1)
				{
					case MA_ACTIVATE:
						{ IsActivateWindow = true; IsMousemessageDiscard_ =false;}
						break;
					case MA_ACTIVATEANDEAT:
						{ IsActivateWindow = true; IsMousemessageDiscard_ = true; }
					break;
					case MA_NOACTIVATE:
						{ IsActivateWindow = false; IsMousemessageDiscard_ = false; }
					break;
					case MA_NOACTIVATEANDEAT:
						{ IsActivateWindow = false; IsMousemessageDiscard_ = true; }
					break;
				}

				
				if ( IsActivateWindow )
				{
					ret = D2DControls::DefWndProc(this,msg,(INT_PTR)&mosue_wp_,lp); // send LRButtonDown message
					mosue_wp_.ptprv = FPointF(lp);
					break;
				}

				auto s2 = GetCapture();

				if ( r1 == 0 && s1 != nullptr && s2 == nullptr)
				{
					s1 = nullptr;
					// captureがリリースされたので、もう一度
				}
				else
					break;
			}

			

			redraw_ = true;
		}
		break;
		case WM_KEYDOWN:			
		{
			if (focus_ == nullptr) return 1;
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			redraw_ = true;

			switch (arg->VirtualKey)
			{
				case Windows::System::VirtualKey::Escape:
				{
					DoDestroy();
					tooltip_->Hide();

					ret = D2DControls::DefWndProc(this, msg, wp, lp);
					
					return 1;
				}
				break;
			}

			if (gui_lock_) return 1;
						

			if (focus_)
				ret = focus_->WndProc(this, msg, wp, lp);
			redraw_ = true;
		}	
		break;
		case WM_KEYUP:
		case WM_CHAR:
		{
			if ( focus_ == nullptr ) return 1;
			if (gui_lock_) return 1;
			//ret = D2DControls::DefWndProc(this,msg,wp,lp);

			focus_->WndProc(this, msg, wp, lp);
			redraw_ = true;
		}	
		break;
		case WM_LBUTTONUP:	
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONUP:	
		case WM_RBUTTONDBLCLK:
		{
			if (gui_lock_) return 1;

			if ( !IsMousemessageDiscard_)
				ret = D2DControls::DefWndProc(this,msg,(INT_PTR)&mosue_wp_,lp);
			redraw_ = true;
		}	
		break;		
		case WM_SIZE:
		{
			Windows::UI::Core::WindowSizeChangedEventArgs^ args = (Windows::UI::Core::WindowSizeChangedEventArgs^)lp;

			rc_.top = 0;
			rc_.left = 0;
			rc_.right = args->Size.Width;
			rc_.bottom = args->Size.Height;
						
			D2DControls::DefWndProc(this,msg,wp,lp);
		}	
		break;
		case WM_D2D_COMMAND:
		{
			if ( wp == WP_D2D_MAINFRAME_CLOSE )
			{
				this->Close();

			}


		}
		break;
		case WM_D2D_INIT_UPDATE:
		{
			TimerSetup();

			rc_.SetSize(*(FSizeF*)wp);

			D2DControls::DefWndProc( this, msg, wp, lp );
		}
		break;


		case WM_D2D_NCHITTEST:
		{
			LOGPT(pt3, wp)

			if (rc_.PtInRect(pt3))
				ret = HTCLIENT;
			else
				ret = D2DControls::DefWndProc(this, msg, wp, lp);
		}
		break;
		case WM_D2D_MOUSEACTIVATE:
		{
			LOGPT(pt3, wp)

			if (rc_.PtInRect(pt3))
				ret = MA_ACTIVATE;
			else
				ret = D2DControls::DefWndProc(this, msg, wp, lp);
		}
		break;
		case WM_D2D_APP_INIT:
		{
			
			SendMessage(WM_D2D_INIT_UPDATE, wp, nullptr);

		}
		break;
		case WM_D2D_SHOW_TOOLTIP:
		{
			WParameterToolTip* t = (WParameterToolTip*)wp;
			if (t && tooltip_->Show(this, t->str, t->pt ))
				ret = 1;
		}
		break;
		case WM_D2D_HIDE_TOOLTIP:
		{
			if ( tooltip_->Hide())
				ret = 1;
		}
		break;
		case WM_D2D_UI_LOCK:
		{
			gui_lock_ = true;
			ret = 1;
		}
		break;
		case WM_D2D_UI_UNLOCK:
		{
			gui_lock_ = false;
			ret = 1;
		}
		break;

		default:
		{
			ret = D2DControls::DefWndProc( this, msg, wp, lp );
		}
		break;
	}
	return ret;
}
void D2DMainWindow::SetCursor(int idx)
{
	static int now = -1;

	if ( now != idx )
	{
		now = idx;
		CoreWindow::GetForCurrentThread()->PointerCursor = cursor_[now];
	}
}

void D2DMainWindow::TimerSetup()
{
	Windows::Foundation::TimeSpan tm;
	tm.Duration = (10*1000)*1000*1; // 1second
	
	Platform::Agile<Windows::UI::Core::CoreWindow> ag(Windows::UI::Core::CoreWindow::GetForCurrentThread());

	auto gui_timer1 = [ag,this](Windows::System::Threading::ThreadPoolTimer^ timer)
	{		
		_ASSERT ( ag->Dispatcher );
		
		ag->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]()
		{
			// GUI thread
			if ( !timerfuncs_.empty())
			{				
				for( auto it = timerfuncs_.begin(); it != timerfuncs_.end();  ) // it++はNG
				{
					bool bComplted = false;
					auto& func = (*it);

					func(0, &bComplted ); // 1sec call fun

					if ( bComplted )
					{					 
						it = timerfuncs_.erase(it);
					}
					else
						it++;
				}
			}
		}));		
	};
	ThreadPoolTimer::CreatePeriodicTimer(ref new TimerElapsedHandler(gui_timer1), tm);
}

void D2DMainWindow::DoDestroy()
{
	pre_death_objects_.clear(); // del memory
}
void D2DMainWindow::SampleRender(LPCWSTR msg)
{
	FRectF rca(0,0,1000,200);

	auto cxt = cxt_.cxt;

	cxt->BeginDraw();
	{
		D2D1_MATRIX_3X2_F mat = D2D1::Matrix3x2F::Identity();

		cxt->SetTransform(mat);
		cxt->Clear(D2D1::ColorF(D2D1::ColorF::LightGray));

		cxt->DrawText(msg, wcslen(msg), cxt_.textformat, rca, cxt_.bluegray);
	}
	cxt->EndDraw();
}

void D2DMainWindow::AddDeath( std::shared_ptr<D2DControl> obj )
{
	pre_death_objects_.push_back( obj );
}
void D2DMainWindow::AliveMeter(Windows::System::Threading::ThreadPoolTimer^ timer)
{	
	//WndProc(0,WM_D2D_IDLE,0,nullptr);

	TRACE( L"AliveMeter\n" );
}
int D2DMainWindow::SendMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	_ASSERT(::GetCurrentThreadId() == gui_thread_id_);
	
	return WndProc( this, message, wp, lp );
}

void D2DMainWindow::SetFocus(D2DControl* focus)
{
	int ret;

//	if (focus_ == focus ) 
//		return;


	if ( focus_ )
		ret = focus_->WndProc(this, WM_D2D_KILLFOCUS, (INT_PTR)0, nullptr);
	
	focus_ = focus;

	
	if (focus_)
	{
		ret = focus_->WndProc(this, WM_D2D_SETFOCUS, (INT_PTR)focus, nullptr);
	}
	else if (focus_ == nullptr && GetCapture())
		ReleaseCapture();

}


D2DControl* D2DMainWindow::FindControl(LPCWSTR name )
{
	return hub_[name]; // when not found, return nullptr.
}

void D2DMainWindow::ReSize()
{
	FSizeF sz = rc_.Size();
	
	DefPaintWndProc(this,WM_SIZE, (INT_PTR)&sz,nullptr); // 全体にWM_SIZEを

}
FRectF D2DMainWindow::GetClientRect()
{
	return rc_;
}
///////////////////////////////////////////
void D2DMainWindow::BAddCapture(D2DCaptureObject* cap)
{
	if ( test_cap_.empty())
		test_cap_.push(cap);
	else if ( test_cap_.top() != cap )
	{
		auto c = test_cap_;

		// stack内に同じobjectがある場合は無視する
		while( !c.empty())
		{
			if ( c.top() == cap )
				return;

			c.pop();
		}

		test_cap_.push(cap);
	}
}

D2DCaptureObject* D2DMainWindow::BGetCapture()
{
	auto p = ( test_cap_.empty() ? nullptr : test_cap_.top());
	return p;
}

void D2DMainWindow::BReleaseCapture(D2DCaptureObject* target)
{	
	if ( target == nullptr )
		test_cap_.clear(); // all release capture
	else
	{
		bool br = false;

		while( !test_cap_.empty() )
		{
			auto p = test_cap_.top();

			if ( target == p )
				br = true;

			test_cap_.pop();

			if ( br )
				break; // 残りはreleaseしない
		}
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CToolTipImplement::Show(D2DMainWindow* d, LPCWSTR str, FPointF pt)
{
	if (layout_ == nullptr && 5000 < ::GetTickCount64()-tick_ )
	{
		auto tf2 = d->cxt()->cxtt.textformat.p;
		auto wf = d->cxt()->cxtt.wfactory;
		if ( S_OK== wf->CreateTextLayout(str, (UINT32)wcslen(str), tf2, 1000, 1000, &layout_))
		{
			DWRITE_TEXT_METRICS tm;
			layout_->GetMetrics(&tm);
		
			FRectFBoxModel rc( pt.x, pt.y , FSizeF(0, 0));
			rc.Padding_.SetRL(10);
			rc.Padding_.SetTB(10);
			rc.SetContentSize(FSizeF(tm.width, tm.height));
		
			rc_ = rc;
			tick_ = ::GetTickCount64();
			return true;
		}
	}
	return false;
}
bool CToolTipImplement::Hide()
{
	if (layout_)
	{
		layout_.Release();
		tick_ = ::GetTickCount64();
		return true;
	}
	return false;
}
void CToolTipImplement::Draw(D2DMainWindow* d)
{
	if (layout_ == nullptr) return;
	auto& cxt = *(d->cxt());

	FRectF rcb = rc_.GetPaddingRect();
	cxt.cxt->FillRectangle(rcb, cxt.blue);

	rcb = rc_.GetContentRect();
	cxt.cxt->DrawTextLayout(rcb.LeftTop(), layout_, cxt.white, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP);
}
