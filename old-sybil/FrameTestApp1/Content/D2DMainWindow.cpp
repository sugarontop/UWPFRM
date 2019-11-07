#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DTextbox.h"
#include "D2DWindowMessage.h"

using namespace Windows::System::Threading;
using namespace Windows::UI::Core;

using namespace V4;


Windows::UI::Core::CoreCursor^ D2DMainWindow::cursor_[5];

D2DMainWindow::D2DMainWindow():back_color_(D2RGB(195,195,195))
{
	parent_control_ = &dumy_;
	
	imebridge_ = nullptr;
	gui_thread_id_ = ::GetCurrentThreadId();
	lock_.Init();


	cursor_[CURSOR_ARROW] = ref new CoreCursor(CoreCursorType::Arrow,0);
	cursor_[CURSOR_IBeam] = ref new CoreCursor(CoreCursorType::IBeam,0);
	cursor_[CURSOR_Hand] = ref new CoreCursor(CoreCursorType::Hand,0);
	cursor_[CURSOR_SizeWestEast] = ref new CoreCursor(CoreCursorType::SizeWestEast,0);
	cursor_[CURSOR_SizeNorthwestSoutheast] = ref new CoreCursor(CoreCursorType::SizeNorthwestSoutheast,0);

	SetCursor(CURSOR_ARROW);

}

int D2DMainWindow::PostWndProc( int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp )
{
	thread_scope sc(lock_);
	int ret = 0;
	//if ( sc.lock(lock_) )
	{
		for( auto& itm : post_message_ar_ )
		{
			for( auto& it : controls_ )
			{
				if ( 0 != ( ret= it->WndProc(this, itm.message, itm.wp, nullptr)) )
					break;
			}			
		}
		post_message_ar_.clear();	
	}
	return ret;
}
int D2DMainWindow::WndProc(D2DWindow* parent, int msg, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{	
	if ( !post_message_ar_.empty())
	{
		PostWndProc( msg, wp, lp );
	}
	
	int ret = 0;
	switch( msg )
	{		
		case WM_PAINT:
		{
			//ColorPtr::cxt_ = cxt_.cxt.p;


				DWORD dt = ::GetTickCount();
				
	

			ID2D1DeviceContext* cxt = (ID2D1DeviceContext*)cxt_.cxt.p;  // see. void D2DContext::Init
			
			
			cxt_.tickcount_ = ::GetTickCount();
			redraw_ = false;
			
			cxt->SaveDrawingState( cxt_.m_stateBlock);
			cxt->BeginDraw();
			D2D1_MATRIX_3X2_F mat = Matrix3x2F::Identity();

			
			cxt->SetTransform(mat);
			cxt->Clear(back_color_);

			DefPaintWndProc( this, WM_PAINT,0,nullptr );
	
			
#ifdef _DEBUG
			D2DMat xm;
			cxt->GetTransform(&xm);
			_ASSERT( xm._31 == 0 && xm._32 == 0 );
#endif

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
			ret = D2DControls::DefWndProc(this,msg,(INT_PTR)&mosue_wp_,lp);
			mosue_wp_.move_ptprv = FPointF(lp);
		}
		break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			auto textbox = (D2DControl*)(imebridge_->GetTarget());

			mosue_wp_.pt = mosue_wp_.move_ptprv = FPointF(lp);
			
			ret = D2DControls::DefWndProc(this,msg,(INT_PTR)&mosue_wp_,lp);

			mosue_wp_.ptprv = FPointF(lp);
			redraw_ = true;
		}
		break;
		case WM_KEYDOWN:			
		{
			Windows::UI::Core::KeyEventArgs^ arg = (Windows::UI::Core::KeyEventArgs^)lp;
			switch( arg->VirtualKey )
			{
				case Windows::System::VirtualKey::Escape:
				{						
					DoDestroy();
				}
			}

			ret = D2DControls::DefWndProc(this,msg,wp,lp);
			redraw_ = true;
		}	
		break;
		case WM_KEYUP:
		case WM_CHAR:
		{
			ret = D2DControls::DefWndProc(this,msg,wp,lp);
			redraw_ = true;
		}	
		break;
		case WM_LBUTTONUP:	
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONUP:	
		case WM_RBUTTONDBLCLK:
		{
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
						
			DefPaintWndProc(this,msg,wp,lp);
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

			D2DControls::DefWndProc( this, msg, wp, lp );
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
	pre_death_objects_.clear(); // del from memory

	return;

	//std::vector<std::shared_ptr<D2DControl>> temp;

	//for( auto& it : pre_death_objects_ )
	//{
	//	if ( it->GetStat() & STAT::CAPTURED_LOCK ) // 保留
	//		temp.push_back(it);
	//}

	//pre_death_objects_.clear(); // del from memory


	//pre_death_objects_ = temp;


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
	return WndProc( this, message, wp, lp );
}
int D2DMainWindow::PostMessage(int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	// ここは別スレッドから呼び出されるので、ロックかけてメッセージをストックする

	thread_scope sc(lock_);

	PostMessageStruct st;
	st.message = message;
	st.wp = wp;
	post_message_ar_.push_back( st );
			
	return 0;
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



