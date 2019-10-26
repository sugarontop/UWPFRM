#include "pch.h"
#include "App2Main.h"
#include "Common\DirectXHelper.h"
#include "Entry.h"
using namespace App2;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// アプリケーションの読み込み時にアプリケーション資産を読み込んで初期化します。
App2Main::App2Main(const std::shared_ptr<DX::DeviceResources>& deviceResources, float cx, float cy, D2CoreTextBridge& pb, bool* closeflg) :
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


	OnEntry(this, FSizeF(cx, cy), imebridge_);

	
}

App2Main::~App2Main()
{
	OnDeviceLost();

	m_deviceResources->RegisterDeviceNotify(nullptr);
}

//ウィンドウのサイズが変更される (デバイスの方向が変更されるなど) 場合に、 アプリケーションの状態を更新します。
void App2Main::CreateWindowSizeDependentResources()
{

}

// アプリケーション状態をフレームごとに 1 回更新します。
void App2Main::Update()
{
	// シーン オブジェクトを更新します。
	//m_timer.Tick([&]()
	//{
	//	// TODO: これをアプリのコンテンツの更新関数で置き換えます。
	//	m_sceneRenderer->Update(m_timer);
	//	m_fpsTextRenderer->Update(m_timer);
	//});
}

// 現在のアプリケーション状態に応じて現在のフレームをレンダリングします。
// フレームがレンダリングされ、表示準備が完了すると、true を返します。
bool App2Main::Render()
{
	//SampleRender(_u("Hello World"));

	WndProc(0, WM_PAINT, 0, nullptr);

	return true;
}

// デバイス リソースを解放する必要が生じたことをレンダラーに通知します。
void App2Main::OnDeviceLost()
{
	cxt_.DestroyRenderTargetResource();
	OnDXDeviceLost();
}

// デバイス リソースの再作成が可能になったことをレンダラーに通知します。
void App2Main::OnDeviceRestored()
{
	CreateWindowSizeDependentResources();
	cxt_.CreateDeviceResources(m_deviceResources->GetD2DDeviceContext());
	OnDXDeviceRestored();


}

void App2Main::Close()
{

}



