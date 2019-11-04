#include "pch.h"
#include "App.h"

#include <ppltasks.h>

using namespace App3;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;




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

	m_deviceResources->SetWindow(window);




	//////////////////////////////////////////////////////////////////////////////

	AddMouseEventHandler(window);
	AddKeyboardEventHandler(window);

	AddImeEventHandler(Windows::UI::Text::Core::CoreTextServicesManager::GetForCurrentView());


}

// シーンのリソースを初期化するか、以前に保存したアプリ状態を読み込みます。
void App::Load(Platform::String^ entryPoint)
{
	if (m_main == nullptr)
	{
		m_main = std::unique_ptr<App3Main>(new App3Main(m_deviceResources, m_ini_cx, m_ini_cy, imeBridge_, &m_windowClosed));
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

			if (m_main->Render())
			{
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
#include "..\libsrc\Image.h"
void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() は CoreWindow がアクティブ化されるまで起動されません。
	CoreWindow::GetForCurrentThread()->Activate();

	auto corewindow  = CoreWindow::GetForCurrentThread();
	auto rc = corewindow->Bounds;


	//auto loader = Windows::ApplicationModel::Resources::ResourceLoader::GetForCurrentView("resources1"); 
	//auto nm = loader->GetString("name");
	//nm = loader->GetString("target1_img");


	
	



	//if (D2DResource::LoadImage(cxt, imagename, &bmp))


	FSizeF sz1( rc.Right -rc.Left, rc.Bottom-rc.Top );
	m_main->WndProc(0, WM_D2D_APP_INIT, (INT_PTR)&sz1, nullptr);
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


	FSizeF sz1(sz.Width, sz.Height);
	m_main->WndProc(0, WM_SIZE, (INT_PTR)&sz1, args);
	m_main->redraw_ = true;
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

// DisplayInformation イベント ハンドラー。

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	// 注意: 高解像度のデバイス用にスケーリングされている場合は、取得した LogicalDpi の値がアプリの有効な DPI と一致しない場合があります。
	// DPI が DeviceResources 上に設定された場合、
	// 常に GetDpi メソッドを使用してそれを取得する必要があります。
	// 詳細については、DeviceResources.cpp を参照してください。
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

//////////////////////////////////////////////////////////////////////////////////
void App::AddMouseEventHandler(Windows::UI::Core::CoreWindow^ window)
{
	window->PointerPressed += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerPressed);
	window->PointerMoved += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerMoved);
	window->PointerReleased += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerReleased);


}
void App::OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	FPointF pt = FPointF(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);

	if (args->CurrentPoint->Properties->IsLeftButtonPressed)
	{
		// 0.5s double click event
		if ((args->CurrentPoint->Timestamp < dbl_.time + 1000 * 500 * 1) && pt == dbl_.pt)
			m_main->WndProc(0, WM_LBUTTONDBLCLK, 0, args);
		else
			m_main->WndProc(0, WM_LBUTTONDOWN, 0, args);


		dbl_.pt = pt;
		dbl_.time = args->CurrentPoint->Timestamp;
		dbl_.bLeft = 1;
	}
	else if (args->CurrentPoint->Properties->IsRightButtonPressed)
	{
		// 0.5s double click event
		if ((args->CurrentPoint->Timestamp < dbl_.time + 1000 * 500 * 1) && pt == dbl_.pt)
			m_main->WndProc(0, WM_RBUTTONDBLCLK, 0, args);
		else
			m_main->WndProc(0, WM_RBUTTONDOWN, 0, args);

		dbl_.pt = pt;
		dbl_.time = args->CurrentPoint->Timestamp;
		dbl_.bLeft = -1;
	}
}

void App::OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	m_main->WndProc(0, WM_MOUSEMOVE, 0, args);
}
void App::OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	if (dbl_.bLeft == 1)
		m_main->WndProc(0, WM_LBUTTONUP, 0, args);
	else if (dbl_.bLeft == -1)
		m_main->WndProc(0, WM_RBUTTONUP, 0, args);
}

//////////////////////////////////////////////////////////////////////////////////
void App::AddKeyboardEventHandler(Windows::UI::Core::CoreWindow^ window)
{
	window->KeyDown += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::KeyEventArgs^>(this, &App::OnKeyDown);
	window->KeyUp += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::KeyEventArgs^>(this, &App::OnKeyUp);
	window->CharacterReceived += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::CharacterReceivedEventArgs^>(this, &App::OnCharacterReceived);
}
void App::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
	m_main->WndProc(0, WM_KEYDOWN, 0, args);
}
void App::OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
	m_main->WndProc(0, WM_KEYUP, 0, args);
}
void App::OnCharacterReceived(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CharacterReceivedEventArgs^ args)
{
	m_main->WndProc(0, WM_CHAR, 0, args);
}
//////////////////////////////////////////////////////////////////////////////////

#include "../libsrc/D2DImeApp.h"

void App::AddImeEventHandler(Windows::UI::Text::Core::CoreTextServicesManager^ ctsMgr)
{
	ctsMgr->InputLanguageChanged += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextServicesManager^, Platform::Object^>(this, &App::OnInputLanguageChanged);
	Windows::UI::Text::Core::CoreTextEditContext^ edcxt = ctsMgr->CreateEditContext();

	edcxt->CompositionStarted += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext^, Windows::UI::Text::Core::CoreTextCompositionStartedEventArgs^>(this, &App::OnCompositionStarted);
	edcxt->CompositionCompleted += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext^, Windows::UI::Text::Core::CoreTextCompositionCompletedEventArgs^>(this, &App::OnCompositionCompleted);
	edcxt->FocusRemoved += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext^, Platform::Object^>(this, &App::OnFocusRemoved);
	edcxt->LayoutRequested += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext^, Windows::UI::Text::Core::CoreTextLayoutRequestedEventArgs^>(this, &App::OnLayoutRequested);
	edcxt->SelectionRequested += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext^, Windows::UI::Text::Core::CoreTextSelectionRequestedEventArgs^>(this, &App::OnSelectionRequested);
	edcxt->SelectionUpdating += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext^, Windows::UI::Text::Core::CoreTextSelectionUpdatingEventArgs^>(this, &App::OnSelectionUpdating);
	edcxt->TextRequested += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext^, Windows::UI::Text::Core::CoreTextTextRequestedEventArgs^>(this, &App::OnTextRequested);
	edcxt->TextUpdating += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext^, Windows::UI::Text::Core::CoreTextTextUpdatingEventArgs^>(this, &App::OnTextUpdating);
	edcxt->FormatUpdating += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Text::Core::CoreTextEditContext^, Windows::UI::Text::Core::CoreTextFormatUpdatingEventArgs^>(this, &App::OnFormatUpdating);

	imeBridge_.Set(edcxt);
}
void App::OnInputLanguageChanged(Windows::UI::Text::Core::CoreTextServicesManager^ sender, Platform::Object^ args)
{
	App_OnInputLanguageChanged(imeBridge_, sender, args);

}
void App::OnTextUpdating(Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextTextUpdatingEventArgs^ args)
{
	App_OnTextUpdating(imeBridge_, sender, args);
	m_main->redraw_ = true;
}

void App::OnFormatUpdating(Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextFormatUpdatingEventArgs^ args)
{
	App_OnFormatUpdating(imeBridge_, sender,args );
	m_main->redraw_ = true;
}

void App::OnCompositionStarted(Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextCompositionStartedEventArgs^ args)
{
	App_OnCompositionStarted(imeBridge_, sender, args);
}
void App::OnCompositionCompleted(Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextCompositionCompletedEventArgs^ args)
{
	App_OnCompositionCompleted(imeBridge_, sender, args);
	m_main->redraw_ = true;
}

void App::OnFocusRemoved(Windows::UI::Text::Core::CoreTextEditContext^ sender, Platform::Object^ args)
{
	App_OnFocusRemoved(imeBridge_, sender, args);
}

void App::OnLayoutRequested(Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextLayoutRequestedEventArgs^ args)
{
	App_OnLayoutRequested(imeBridge_, sender, args);
}

void App::OnSelectionRequested(Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextSelectionRequestedEventArgs^ args)
{
	App_OnSelectionRequested(imeBridge_, sender, args);
}

void App::OnSelectionUpdating(Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextSelectionUpdatingEventArgs^ args)
{
	App_OnSelectionUpdating(imeBridge_, sender, args);
}
void App::OnTextRequested(Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextTextRequestedEventArgs^ args)
{
	App_OnTextRequested(imeBridge_, sender, args);
}



