#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
//////////////////////////////////////////////
#include "../libsrc/D2Dcontext.h"
#include "../libsrc/D2DUniversalControl.h"
using namespace V4;

// Direct2D および 3D コンテンツを画面上でレンダリングします。
namespace App3
{
	class App3Main : public DX::IDeviceNotify, public D2DMainWindow
	{
	public:
		App3Main(const std::shared_ptr<DX::DeviceResources>& deviceResources, float cx, float cy, D2CoreTextBridge& pb, bool* closeflg);
		~App3Main();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();
		virtual void Close() override;


		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// デバイス リソースへのキャッシュされたポインター。
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

	public:
		virtual D2DContext* cxt() override { return &cxt_; }
		virtual DX::DeviceResources* GetResource() { return m_deviceResources.get(); }
	public:
		D2CoreTextBridge* imebridge_;
		bool* closeflg_;
	};
}