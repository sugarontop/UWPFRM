#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"

//////////////////////////////////////////////
#include "content\d2dcontext.h"
#include "content\d2dUniversalControl.h"
using namespace V4;
//////////////////////////////////////////////


// Direct2D および 3D コンテンツを画面上でレンダリングします。
namespace FrameTestApp1
{
	class FrameTestApp1Main : public DX::IDeviceNotify, public D2DMainWindow
	{
	public:
		FrameTestApp1Main(const std::shared_ptr<DX::DeviceResources>& deviceResources,float cx, float cy, D2CoreTextBridge& pb, bool* closeflg); // <--
		~FrameTestApp1Main();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

		virtual void Close() override ;

	public:
		// デバイス リソースへのキャッシュされたポインター。
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: これを独自のコンテンツ レンダラーで置き換えます。
		/*std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;*/

		// ループ タイマーをレンダリングしています。
		DX::StepTimer m_timer;
	
	public :// <--
		virtual D2DContext* cxt(){ return &cxt_; }
		virtual DX::DeviceResources* GetResource(){ return m_deviceResources.get(); }
	public :// <--
		D2CoreTextBridge* imebridge_;
		bool* closeflg_;
	};
}