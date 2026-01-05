#pragma once

#include "events/application_event.h"

namespace penumbra
{
	class CGraphicsContext
	{
	public:
		CGraphicsContext();
		~CGraphicsContext() = default;

		void SwapBuffers() const;
		void SetVSync(bool bEnabled) { m_nSwapInterval = bEnabled == true ? 1 : 0; }

		ComPtr<ID3D11Device> GetDevice() const { return m_spDevice; }
		ComPtr<ID3D11DeviceContext> GetContext() const { return m_spContext; }
		ComPtr<IDXGIFactory2> GetDXGIFactory() const { return m_spFactory; }
		bool GetTearing() const { return m_bTearing; }
		void SetCurrentSwapchain(ComPtr<IDXGISwapChain> spSwapChain) { m_spSwapChain = spSwapChain; }
	private:
		ComPtr<IDXGISwapChain> m_spSwapChain;

		ComPtr<ID3D11Device> m_spDevice;
		ComPtr<ID3D11DeviceContext> m_spContext;
		ComPtr<IDXGIAdapter> m_spDXGIAdapter;
		ComPtr<IDXGIDevice> m_spDXGIDevice;

		ComPtr<IDXGIFactory2> m_spFactory;

		ComPtr<ID3D11Texture2D> m_spBackBuffer;

		UINT m_nSwapInterval = 1;
		bool m_bTearing = false;
	};
}