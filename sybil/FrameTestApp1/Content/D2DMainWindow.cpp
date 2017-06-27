#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"

namespace V4 {

D2DMainWindow::D2DMainWindow():back_color_(D2RGB(195,195,195))
{
	parent_control_ = &dumy_;
	
	imebridge_ = nullptr;
	gui_thread_id_ = ::GetCurrentThreadId();
	lock_.Init();
}
D2DCaptureObject* D2DMainWindow::SetTopCapture(D2DCaptureObject* cap)
{ 
	auto t = cap_; 
	cap_=cap; 
	return t;
}
int D2DMainWindow::PostWndProc( int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp )
{
	thread_scope sc;
	int ret = 0;
	if ( sc.lock(lock_) )
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
			ColorPtr::cxt_ = cxt_.cxt.p;


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

			
			TRACE( L"PAINT=%d\n", ::GetTickCount()-dt );
			

			return 0;
		}
		break;		
		case WM_LBUTTONDOWN:
		{
			redraw_ = true;

			auto textbox = (D2DControl*)(imebridge_->GetTarget());
			if ( textbox )
				textbox->UnActivate();
			
			
			ret = D2DControls::DefWndProc(this,msg,wp,lp);

		}
		break;
		case WM_LBUTTONUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		{
			redraw_ = true;
			
			ret = D2DControls::DefWndProc(this,msg,wp,lp);
		}	
		break;
		
		case WM_SIZE:
		{
			Windows::UI::Core::WindowSizeChangedEventArgs^ args = (Windows::UI::Core::WindowSizeChangedEventArgs^)lp;

			rc_.top = 0;
			rc_.left = 0;
			rc_.right = args->Size.Width;
			rc_.bottom = args->Size.Height;
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
	thread_scope sc(lock_);

	PostMessageStruct st;
	st.message = message;
	st.wp = wp;
	post_message_ar_.push_back( st );
			
	return 0;
}



}


