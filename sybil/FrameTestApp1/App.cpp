﻿#include "pch.h"
#include "App.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "content/D2DCommon.h"
#include "content/D2DMisc.h"
using namespace V4;

#define MAINFRAME FrameTestApp1Main

/////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace FrameTestApp1;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Text::Core;

// main 関数は、IFrameworkView クラスを初期化する場合にのみ使用します。
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

App::App() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

// IFrameworkView の作成時に最初のメソッドが呼び出されます。
void App::Initialize(CoreApplicationView^ applicationView)
{
	// アプリ ライフサイクルのイベント ハンドラーを登録します。この例にはアクティブ化が含まれているため、
	// CoreWindow をアクティブにし、ウィンドウで描画を開始できます。
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &App::OnResuming);

	//この時点では、デバイスにアクセスできます。
	// デバイスに依存するリソースを作成できます。
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	
}



//CoreWindow オブジェクトが作成 (または再作成) されるときに呼び出されます。
void App::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += 
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

	window->Closed += 
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);

	// restruct WM_MESSAGE.　mouse message
	
	window->KeyDown += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::KeyEventArgs ^>(this, &FrameTestApp1::App::OnKeyDown);
	window->KeyUp += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::KeyEventArgs ^>(this, &FrameTestApp1::App::OnKeyUp);
	window->CharacterReceived += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^,Windows::UI::Core::CharacterReceivedEventArgs ^>(this,&FrameTestApp1::App::OnCharacterReceived);
	
	window->PointerPressed += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::PointerEventArgs ^>(this, &FrameTestApp1::App::OnPointerPressed);
	window->PointerMoved += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::PointerEventArgs ^>(this, &FrameTestApp1::App::OnPointerMoved);
	window->PointerReleased += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::PointerEventArgs ^>(this, &FrameTestApp1::App::OnPointerReleased);	
	window->PointerWheelChanged += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^,Windows::UI::Core::PointerEventArgs ^>(this,&FrameTestApp1::App::OnPointerWheelChanged);

	m_deviceResources->SetWindow(window);
	
	window->IsInputEnabled = true;
	
	m_ini_cx = window->Bounds.Width;
	m_ini_cy = window->Bounds.Height;
	
	
	
	//
	// Create ime contexts.  Windows::UI::Text::Core!!
	//
	{
		Windows::UI::Text::Core::CoreTextServicesManager^ ctsMgr = CoreTextServicesManager::GetForCurrentView();
		ctsMgr->InputLanguageChanged += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextServicesManager ^,Platform::Object ^>(this,&FrameTestApp1::App::OnInputLanguageChanged);

		CoreTextEditContext^ edcxt = ctsMgr->CreateEditContext();

		edcxt->CompositionStarted += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^,Windows::UI::Text::Core::CoreTextCompositionStartedEventArgs ^>(this,&FrameTestApp1::App::OnCompositionStarted);
		edcxt->CompositionCompleted += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^,Windows::UI::Text::Core::CoreTextCompositionCompletedEventArgs ^>(this,&FrameTestApp1::App::OnCompositionCompleted);
		edcxt->FocusRemoved += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^,Platform::Object ^>(this,&FrameTestApp1::App::OnFocusRemoved);
		edcxt->LayoutRequested += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^,Windows::UI::Text::Core::CoreTextLayoutRequestedEventArgs ^>(this,&FrameTestApp1::App::OnLayoutRequested);
		edcxt->SelectionRequested += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^,Windows::UI::Text::Core::CoreTextSelectionRequestedEventArgs ^>(this,&FrameTestApp1::App::OnSelectionRequested);
		edcxt->SelectionUpdating += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^,Windows::UI::Text::Core::CoreTextSelectionUpdatingEventArgs ^>(this,&FrameTestApp1::App::OnSelectionUpdating);
		edcxt->TextRequested += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^,Windows::UI::Text::Core::CoreTextTextRequestedEventArgs ^>(this,&FrameTestApp1::App::OnTextRequested);
		edcxt->TextUpdating += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^,Windows::UI::Text::Core::CoreTextTextUpdatingEventArgs ^>(this,&FrameTestApp1::App::OnTextUpdating);
		edcxt->FormatUpdating += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext ^,Windows::UI::Text::Core::CoreTextFormatUpdatingEventArgs ^>(this,&FrameTestApp1::App::OnFormatUpdating);
	
		imeBridge_.Set(edcxt);
	}

	
	
}

// シーンのリソースを初期化するか、以前に保存したアプリ状態を読み込みます。
void App::Load(Platform::String^ entryPoint)
{
	if (m_main == nullptr)
	{
		
		m_main = std::unique_ptr<MAINFRAME>(new MAINFRAME(m_deviceResources,m_ini_cx,m_ini_cy, imeBridge_, &m_windowClosed));		


		
	}
}

// このメソッドは、ウィンドウがアクティブになると、呼び出されます。
void App::Run()
{
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			m_main->Update();

			if ( m_main->redraw_  )
			{
				if (m_main->Render())
					m_deviceResources->Present();
			}			
		}
		else
		{
			
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);

		}
	}

}

// IFrameworkView で必要です。
// 終了イベントでは初期化解除は呼び出されません。アプリケーションが前景に表示されている間に
//IFrameworkView クラスが解体されると呼び出されます。
void App::Uninitialize()
{

}

// アプリケーション ライフサイクル イベント ハンドラー。

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() は CoreWindow がアクティブ化されるまで起動されません。
	CoreWindow::GetForCurrentThread()->Activate();


	
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// 遅延を要求した後にアプリケーションの状態を保存します。遅延状態を保持することは、
	//中断操作の実行でアプリケーションがビジー状態であることを示します。
	//遅延は制限なく保持されるわけではないことに注意してください。約 5 秒後に、
	// アプリケーションは強制終了されます。
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
        m_deviceResources->Trim();

		// ここにコードを挿入します。

		deferral->Complete();
	});
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// 中断時にアンロードされたデータまたは状態を復元します。既定では、データと状態は
	// 中断から再開するときに保持されます。このイベントは、アプリが既に終了されている場合は
	//発生しません。

	// ここにコードを挿入します。
}

// ウィンドウ イベント ハンドラー。 

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	Windows::Foundation::Size sz;
	sz.Width = sender->Bounds.Width;
	sz.Height = sender->Bounds.Height;
		
	m_deviceResources->SetLogicalSize(sz);
	m_main->CreateWindowSizeDependentResources();


	FSizeF sz1(sz.Width,sz.Height);

	m_main->WndProc(0,WM_SIZE,(INT_PTR)&sz1, args );

	m_main->redraw_ = true;

}




void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;


}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	//m_main->WndProc(0, WM_DESTROY, 0, args );
	
	m_windowClosed = true;


	TRACE( L"OnWindowClosed\n"); 
}

// DisplayInformation イベント ハンドラー。

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	m_deviceResources->SetDpi(sender->LogicalDpi);
	m_main->CreateWindowSizeDependentResources();
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->CreateWindowSizeDependentResources();
}

void App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	m_deviceResources->ValidateDevice();
}

void FrameTestApp1::App::OnKeyDown(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args)
{
	m_main->WndProc(0, WM_KEYDOWN, 0, args );
}
void FrameTestApp1::App::OnKeyUp(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args)
{
	m_main->WndProc(0, WM_KEYUP, 0, args);
}

void FrameTestApp1::App::OnPointerPressed(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::PointerEventArgs ^args)
{	
	FPointF pt = FPointF( args->CurrentPoint->Position.X,  args->CurrentPoint->Position.Y );

	if ( args->CurrentPoint->Properties->IsLeftButtonPressed )
	{
		// 0.5s double click event
		if ( (args->CurrentPoint->Timestamp < dbl_.time + 1000*500*1 ) &&  pt == dbl_.pt )
			m_main->WndProc(0,WM_LBUTTONDBLCLK, 0, args );
		else
			m_main->WndProc(0,WM_LBUTTONDOWN, 0, args );
	

		dbl_.pt = pt;
		dbl_.time = args->CurrentPoint->Timestamp;
		dbl_.bLeft = 1;
	}
	else if ( args->CurrentPoint->Properties->IsRightButtonPressed )
	{
		// 0.5s double click event
		if ( (args->CurrentPoint->Timestamp < dbl_.time + 1000*500*1 ) &&  pt == dbl_.pt )
			m_main->WndProc(0,WM_RBUTTONDBLCLK, 0, args );
		else
			m_main->WndProc(0,WM_RBUTTONDOWN, 0, args );

		dbl_.pt = pt;
		dbl_.time = args->CurrentPoint->Timestamp;
		dbl_.bLeft = -1;
	}
}

void FrameTestApp1::App::OnPointerMoved(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::PointerEventArgs ^args)
{
	m_main->WndProc(0, WM_MOUSEMOVE, 0,args);
}
void FrameTestApp1::App::OnPointerReleased(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::PointerEventArgs ^args)
{
	if (dbl_.bLeft == 1)
		m_main->WndProc(0, WM_LBUTTONUP, 0, args);
	else if (dbl_.bLeft == -1)
		m_main->WndProc(0, WM_RBUTTONUP, 0, args);
}
void FrameTestApp1::App::OnCharacterReceived(Windows::UI::Core::CoreWindow ^sender,Windows::UI::Core::CharacterReceivedEventArgs ^args)
{
	m_main->WndProc(0, WM_CHAR, 0, args);
}
void FrameTestApp1::App::OnPointerWheelChanged(Windows::UI::Core::CoreWindow ^sender,Windows::UI::Core::PointerEventArgs ^args)
{
	m_main->WndProc(0, WM_MOUSEWHEEL, 0, args);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameTestApp1::App::OnInputLanguageChanged(Windows::UI::Text::Core::CoreTextServicesManager ^sender,Platform::Object ^args)
{
	Windows::Globalization::Language^ lang = sender->InputLanguage;

}



Windows::Foundation::Rect ClientToScreen( FRectF rc )
{
	Windows::Foundation::Rect rcc = CoreWindow::GetForCurrentThread()->Bounds;

	rc.Offset( rcc.Left, rcc.Top );

	Windows::Foundation::Rect r;
	r.X = rc.left;
	r.Height = rc.bottom-rc.top;
	r.Y = rc.top;
	r.Width = rc.right - rc.left;
	return r;	
}



void FrameTestApp1::App::OnSelectionRequested(Windows::UI::Text::Core::CoreTextEditContext ^sender,Windows::UI::Text::Core::CoreTextSelectionRequestedEventArgs ^args)
{
	args->Request->Selection = imeBridge_.GetSelection();

	//TRACE( L"OnSelectionRequested %d \n", args->Request->Selection );
}

std::wstring ReplaceString(std::wstring String1,std::wstring String2,std::wstring String3)// 置換対象, 検索対象,置換え
{
    std::wstring::size_type  Pos( String1.find( String2 ) );
 
    while( Pos != std::string::npos )
    {
        String1.replace( Pos, String2.length(), String3 );
        Pos = String1.find( String2, Pos + String3.length() );
    }
 
    return String1;
}
 

void FrameTestApp1::App::OnTextRequested(Windows::UI::Text::Core::CoreTextEditContext ^sender,Windows::UI::Text::Core::CoreTextTextRequestedEventArgs ^args)
{
	if ( imeBridge_.info_ )
	{
		std::wstring& s = imeBridge_.info_->text;

		int start = args->Request->Range.StartCaretPosition;
		int end = min( args->Request->Range.EndCaretPosition, (int)s.length());

		std::wstring s2 = s.substr( start, end-start );
		
		args->Request->Text = ref new Platform::String( s2.c_str() );
				

		//TRACE( L"OnTextRequested, (%d-%d), %s\n", start,end, s2.c_str() );
	}
}

void FrameTestApp1::App::OnSelectionUpdating(Windows::UI::Text::Core::CoreTextEditContext ^sender,Windows::UI::Text::Core::CoreTextSelectionUpdatingEventArgs ^args)
{
	imeBridge_.SetNewSelection( args->Selection );

	//TRACE( L"OnSelectionUpdating\n" );
}

void FrameTestApp1::App::OnLayoutRequested(Windows::UI::Text::Core::CoreTextEditContext ^sender,Windows::UI::Text::Core::CoreTextLayoutRequestedEventArgs ^args)
{
	// candidateの表示位置
	if ( imeBridge_.info_ )
	{		
		auto rc = imeBridge_.info_->rcTextbox;

		int spos = args->Request->Range.StartCaretPosition;
		int epos = args->Request->Range.EndCaretPosition;
		

		if ( spos > 0 )
		{
			FRectF& rcChar = imeBridge_.info_->rcChar().get()[spos-1];

			FSizeF sz( rcChar.right, 0 );

			D2DMat m(imeBridge_.info_->mat);
			auto szDev = m.LPtoDP( sz );


			rc.Offset( szDev.width, 0 );
		}
						
		args->Request->LayoutBounds->ControlBounds = ClientToScreen(imeBridge_.info_->rcTextbox);
		args->Request->LayoutBounds->TextBounds = ClientToScreen(rc);// ime candidate window position

		//TRACE(L"OnLayoutRequested %d-%d  top=%f,bottom=%f\n", args->Request->Range.StartCaretPosition, args->Request->Range.EndCaretPosition, rc.top, rc.bottom);
	}
}




void FrameTestApp1::App::OnTextUpdating(Windows::UI::Text::Core::CoreTextEditContext ^sender,Windows::UI::Text::Core::CoreTextTextUpdatingEventArgs ^args)
{
	//DWORD dt2 = ::GetTickCount();
	//TRACE( L"tm=%d\n", dt2-dt );
	//dt = dt2;
	//return;

	imeBridge_.SetNewSelection(args->NewSelection);

	LPCWSTR c  = args->Text->Data();
	
	int append_len = wcslen(c);

	int ys = args->Range.StartCaretPosition;
	int ye = args->Range.EndCaretPosition;


	auto str3 = str_remove( imeBridge_.info_->text, ys, ye );
		
	for( int i = 0; i < append_len; i++ )
	{
		if ( ys+i < (int)str3.length() )
			str3 = str_append(str3, ys+i, c[i] ); 
		else
		{
			str3 += c;
			break;
		}
	}

	imeBridge_.info_->text = str3;
	imeBridge_.info_->decoration_end_pos = min( (int)str3.length(), imeBridge_.info_->decoration_end_pos);

	auto& rc = imeBridge_.info_->rcTextbox;
	imeBridge_.UpdateTextRect(rc.Size());

	if ( imeBridge_.info_->ontext_updated_ )
		imeBridge_.info_->ontext_updated_();

	m_main->redraw_ = true;

	//TRACE( L"★OnTextUpdating (%d-%d) \n",  ys, ye );
}

void FrameTestApp1::App::OnFormatUpdating(Windows::UI::Text::Core::CoreTextEditContext ^sender,Windows::UI::Text::Core::CoreTextFormatUpdatingEventArgs ^args)
{	
	//TRACE( L"OnFormatUpdating " );

	int typ = (int)args->UnderlineType->Value;

	
	if ( imeBridge_.info_ )
	{
		imeBridge_.info_->decoration_start_pos = args->Range.StartCaretPosition;
		imeBridge_.info_->decoration_end_pos = args->Range.EndCaretPosition;
		imeBridge_.info_->decoration_typ = typ;

	}

	//::OutputDebugString( V4::Format( L"%d %d typ:%d\n", args->Range.StartCaretPosition, args->Range.EndCaretPosition, typ).c_str() );
	m_main->redraw_ = true;
}

void FrameTestApp1::App::OnCompositionStarted(Windows::UI::Text::Core::CoreTextEditContext ^sender,Windows::UI::Text::Core::CoreTextCompositionStartedEventArgs ^args)
{
	// start convert to jp.

	imeBridge_.CompositionStarted();

	//TRACE( L"OnCompositionStarted\n" );

}
void FrameTestApp1::App::OnCompositionCompleted(Windows::UI::Text::Core::CoreTextEditContext ^sender,Windows::UI::Text::Core::CoreTextCompositionCompletedEventArgs ^args)
{
	// finished converting.

	imeBridge_.CompositionCompleted();
	m_main->redraw_ = true;

	//TRACE( L"OnCompositionCompleted\n" );

}

void FrameTestApp1::App::OnFocusRemoved(Windows::UI::Text::Core::CoreTextEditContext ^sender,Platform::Object ^args)
{
	//TRACE(L"OnFocusRemoved\n");
}



