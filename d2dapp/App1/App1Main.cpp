#include "pch.h"
#include "App1Main.h"
#include "Common\DirectXHelper.h"

using namespace App1;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;



// アプリケーションの読み込み時にアプリケーション資産を読み込んで初期化します。
App1Main::App1Main(const std::shared_ptr<DX::DeviceResources>& deviceResources, float cx, float cy, D2CoreTextBridge& pb, bool* closeflg) :
	m_deviceResources(deviceResources)
{
	// デバイスが失われたときや再作成されたときに通知を受けるように登録します
	m_deviceResources->RegisterDeviceNotify(this);

	static V4::SingletonD2DInstance ins;

	ComPTR<ID2D1Factory> fac = m_deviceResources->GetD2DFactory();
	ComPTR<IDWriteFactory> wfac = m_deviceResources->GetDWriteFactory();
	ComPTR<ID2D1DeviceContext> cxt = m_deviceResources->GetD2DDeviceContext();

	cxt_.Init(ins, fac, wfac, cxt);
	cxt_.CreateDeviceResources(cxt);


	imebridge_ = &pb;
	redraw_ = true;
	rc_.SetSize(cx, cy);

	this->parent_ = this;
	closeflg_ = closeflg;


	//OnEntry(this, FSizeF(cx, cy), imebridge_);

	SendMessage(WM_D2D_INIT_UPDATE, 0, nullptr);
}

App1Main::~App1Main()
{
	// デバイスの通知を登録解除しています

	OnDeviceLost();

	m_deviceResources->RegisterDeviceNotify(nullptr);
}

//ウィンドウのサイズが変更される (デバイスの方向が変更されるなど) 場合に、 アプリケーションの状態を更新します。
void App1Main::CreateWindowSizeDependentResources() 
{
	
}

// アプリケーション状態をフレームごとに 1 回更新します。
void App1Main::Update() 
{
	
}

// 現在のアプリケーション状態に応じて現在のフレームをレンダリングします。
// フレームがレンダリングされ、表示準備が完了すると、true を返します。
bool App1Main::Render() 
{
	SampleRender(_u("Hello World"));

	return true;
}

// デバイス リソースを解放する必要が生じたことをレンダラーに通知します。
void App1Main::OnDeviceLost()
{
	//m_sceneRenderer->ReleaseDeviceDependentResources();
	//m_fpsTextRenderer->ReleaseDeviceDependentResources();

	cxt_.DestroyRenderTargetResource();
	OnDXDeviceLost();
}

// デバイス リソースの再作成が可能になったことをレンダラーに通知します。
void App1Main::OnDeviceRestored()
{
	//m_sceneRenderer->CreateDeviceDependentResources();
	//m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	cxt_.CreateDeviceResources(m_deviceResources->GetD2DDeviceContext());
	OnDXDeviceRestored();
}

void App1Main::Close()
{

}