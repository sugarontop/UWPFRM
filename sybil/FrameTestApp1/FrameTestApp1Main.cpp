#include "pch.h"
#include "FrameTestApp1Main.h"
#include "Common\DirectXHelper.h"
#include "Entry.h"
#include "App.h"
using namespace FrameTestApp1;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// アプリケーションの読み込み時にアプリケーション資産を読み込んで初期化します。
FrameTestApp1Main::FrameTestApp1Main(const std::shared_ptr<DX::DeviceResources>& deviceResources,float cx, float cy, D2CoreTextBridge& pb, bool* closeflg) :
	m_deviceResources(deviceResources)
{
	// デバイスが失われたときや再作成されたときに通知を受けるように登録します
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: これをアプリのコンテンツの初期化で置き換えます。
	/*m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));
	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));*/

	// TODO: 既定の可変タイムステップ モード以外のモードが必要な場合は、タイマー設定を変更してください。
	// 例: 60 FPS 固定タイムステップ更新ロジックでは、次を呼び出します:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/


	static V4::SingletonD2DInstance ins;
	cxt_.Init(ins, m_deviceResources );
	cxt_.CreateDeviceResources(m_deviceResources->GetD2DDeviceContext());


	imebridge_ = &pb;
	redraw_ = true;
	rc_.SetSize( cx,cy );

	this->parent_ = this;
	closeflg_ = closeflg;
		
	
	OnEntry(this, FSizeF(cx,cy), imebridge_);

	SendMessage( WM_D2D_INIT_UPDATE,0,nullptr );
	

}

FrameTestApp1Main::~FrameTestApp1Main()
{
	// デバイスの通知を登録解除しています

	FrameTestApp1Main::OnDeviceLost();

	m_deviceResources->RegisterDeviceNotify(nullptr);
}

void FrameTestApp1Main::Close()
{
	*closeflg_ = true;
}

//ウィンドウのサイズが変更される (デバイスの方向が変更されるなど) 場合に、 アプリケーションの状態を更新します。
void FrameTestApp1Main::CreateWindowSizeDependentResources() 
{
	// TODO: これをアプリのコンテンツのサイズに依存する初期化で置き換えます。
	/*m_sceneRenderer->CreateWindowSizeDependentResources();*/
}

// アプリケーション状態をフレームごとに 1 回更新します。
void FrameTestApp1Main::Update() 
{
	// シーン オブジェクトを更新します。
	m_timer.Tick([&]()
	{
		// TODO: これをアプリのコンテンツの更新関数で置き換えます。
		/*m_sceneRenderer->Update(m_timer);
		m_fpsTextRenderer->Update(m_timer);*/
	});
}

// 現在のアプリケーション状態に応じて現在のフレームをレンダリングします。
// フレームがレンダリングされ、表示準備が完了すると、true を返します。
bool FrameTestApp1Main::Render() 
{
	// 初回更新前にレンダリングは行わないようにしてください。
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	//auto context = m_deviceResources->GetD3DDeviceContext();

	////// ビューポートをリセットして全画面をターゲットとします。
	//auto viewport = m_deviceResources->GetScreenViewport();
	//context->RSSetViewports(1, &viewport);

	////// レンダリング ターゲットを画面にリセットします。
	//ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	//context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	////// バック バッファーと深度ステンシル ビューをクリアします。
	//context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	//context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//// シーン オブジェクトをレンダリングします。
	//// TODO: これをアプリのコンテンツのレンダリング関数で置き換えます。
	//m_sceneRenderer->Render();
	//m_fpsTextRenderer->Render();


	WndProc(0,WM_PAINT,0,nullptr);

	return true;
}

// デバイス リソースを解放する必要が生じたことをレンダラーに通知します。
void FrameTestApp1Main::OnDeviceLost()
{

	/*m_sceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();*/


	cxt_.DestroyRenderTargetResource();

	this->OnDXDeviceLost();


}

// デバイス リソースの再作成が可能になったことをレンダラーに通知します。
void FrameTestApp1Main::OnDeviceRestored()
{
	//m_sceneRenderer->CreateDeviceDependentResources();
	/*m_fpsTextRenderer->CreateDeviceDependentResources();*/
	//CreateWindowSizeDependentResources();

	cxt_.CreateDeviceResources(m_deviceResources->GetD2DDeviceContext());
	this->OnDXDeviceRestored();
}
