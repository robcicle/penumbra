#pragma once

#include "events/application_event.h"

namespace penumbra
{
	struct GPUMemoryInfo_t
	{
		// Local is dedicated VRAM (discrete GPUs).
		// Non-Local is shared/system memory the GPU can use
		uint64_t m_nLocalUsedBytes = 0;
		uint64_t m_nLocalBudgetBytes = 0;

		uint64_t m_nNonLocalUsedBytes = 0;
		uint64_t m_nNonLocalBudgetBytes = 0;
	};

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
	
		// GPU memory stats
		bool IsGPUMemoryQueueSupported() const { return m_bGpuMemoryQuerySupported; }
		void UpdateGPUMemoryInfo();
		GPUMemoryInfo_t GetGPUMemoryInfo() const { return m_GPUMemoryInfo; }
	private:
		ComPtr<IDXGISwapChain> m_spSwapChain;

		ComPtr<ID3D11Device> m_spDevice;
		ComPtr<ID3D11DeviceContext> m_spContext;
		ComPtr<IDXGIAdapter> m_spDXGIAdapter;
		ComPtr<IDXGIDevice> m_spDXGIDevice;

		ComPtr<IDXGIAdapter3> m_spDXGIAdapter3;
		bool m_bGpuMemoryQuerySupported;
		GPUMemoryInfo_t m_GPUMemoryInfo{};

		ComPtr<IDXGIFactory2> m_spFactory;

		ComPtr<ID3D11Texture2D> m_spBackBuffer;

		UINT m_nSwapInterval = 1;
		bool m_bTearing = false;

		// Simple throttling
		uint64_t m_nLastGPUMemQueryQPC = 0;
		uint64_t m_nQPCFreq = 0;
	};
}