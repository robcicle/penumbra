#include "ppch.h"
#include "renderer/graphics_context.h"

#include "renderer/renderer.h"

#include <dxgi1_5.h>
#include <core/timer.h>
#include <core/application.h>

namespace penumbra
{
	namespace Utils
	{
		static std::string WStringToUTF8(const std::wstring& ws)
		{
			if (ws.empty())
				return {};

			// Get the size needed for the UTF-8 string
			const int size = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(),
				static_cast<int>(ws.size()),
				nullptr, 0, nullptr, nullptr);

			// Perform the conversion
			std::string result(size, '\0');

			// 2nd call to actually convert the string
			WideCharToMultiByte(CP_UTF8, 0, ws.c_str(),
				static_cast<int>(ws.size()),
				result.data(), size,
				nullptr, nullptr);

			return result;
		}

		static const char* VendorName(UINT vendorId)
		{
			switch (vendorId)
			{
			case kVendorIdNVIDIA: return kVendorNVIDIA;
			case kVendorIdAMD:    return kVendorAMD;
			case kVendorIdIntel:  return kVendorIntel;
			default:              return kVendorUnknown;
			}
		}
	}

	CGraphicsContext::CGraphicsContext()
	{
		PENUMBRA_PROFILE_FUNC();

		// Factory and Adapter setup.
		{
			PENUMBRA_PROFILE_SCOPE(kGraphicsCreateDXGIFactoryProfileName);

			HRESULT hr = S_OK;

			// Create DXGI Factory
			hr = CreateDXGIFactory1(IID_PPV_ARGS(m_spFactory.GetAddressOf()));
			PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "GraphicsContext: Failed to create DXGIFactory!");

			// Simple check to see if the system supports tearing in this case, 
			// the user can disable VSync. Otherwise, they can still disable it
			// but D3D11 just won't abide to the prefered swap interval.
			ComPtr<IDXGIFactory5> f5;
			if (SUCCEEDED(m_spFactory.As(&f5))) {
				BOOL tearing = (BOOL)m_bTearing;
				if (SUCCEEDED(f5->CheckFeatureSupport(
					DXGI_FEATURE_PRESENT_ALLOW_TEARING,
					&tearing,
					sizeof(tearing))))
				{
					m_bTearing = tearing == TRUE;
				}
			}

			// Enumerate Adapters
			hr = m_spFactory->EnumAdapters(0, m_spDXGIAdapter.GetAddressOf());
			PENUMBRA_CORE_ASSERT(m_spDXGIAdapter || SUCCEEDED(hr), "GraphicsContext: Failed to Enumerate Adapters!");

			// Get Adapter Description
			DXGI_ADAPTER_DESC desc;
			hr = m_spDXGIAdapter->GetDesc(&desc);
			PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "GraphicsContext: Failed to get Adapter's Description!");

			// Log adapter info
			PENUMBRA_CORE_INFO("DirectX 11 Info:");
			PENUMBRA_CORE_INFO("  Vendor: {}", Utils::VendorName(desc.VendorId));
			PENUMBRA_CORE_INFO("  Renderer: {}", Utils::WStringToUTF8(desc.Description));
			PENUMBRA_CORE_INFO("  Video Memory: {} MB", desc.DedicatedVideoMemory / kOneMiB);
		}

		m_bGpuMemoryQuerySupported = false;

		// Check for IDXGIAdapter3 support for GPU memory queries
		{
			if (SUCCEEDED(m_spDXGIAdapter.As(&m_spDXGIAdapter3))) {
				m_bGpuMemoryQuerySupported = true;
			}

			LARGE_INTEGER freq{};
			if (QueryPerformanceFrequency(&freq))
				m_nQPCFreq = static_cast<uint64_t>(freq.QuadPart);
		}

		// Device and Context setup.
		UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef PENUMBRA_DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		const D3D_FEATURE_LEVEL requested = D3D_FEATURE_LEVEL_11_1;

		{
			PENUMBRA_PROFILE_SCOPE("D3D11CreateDevice");

			// Create Device and Context
			HRESULT hr = D3D11CreateDevice(
				m_spDXGIAdapter.Get(),
				D3D_DRIVER_TYPE_UNKNOWN,
				nullptr,
				flags,
				&requested,
				1,
				D3D11_SDK_VERSION,
				m_spDevice.GetAddressOf(),
				nullptr,
				m_spContext.GetAddressOf()
			);

			PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "GraphicsContext: Failed to create D3D11 device!");
		}

		// Validation
		PENUMBRA_CORE_ASSERT(m_spDevice, "GraphicsContext: Failed to create Device!");
		PENUMBRA_CORE_ASSERT(m_spContext, "GraphicsContext: Failed to create DeviceContext!");
	}

	void CGraphicsContext::SwapBuffers() const
	{
		PENUMBRA_PROFILE_FUNC();

		if (!m_spSwapChain)
			return;

		UINT flags = 0;
		if (m_bTearing && m_nSwapInterval == 0)
			flags |= DXGI_PRESENT_ALLOW_TEARING;

		// Present the swap chain
		{
			PENUMBRA_PROFILE_SCOPE("IDXGISwapChain::Present");
			// Include the time taken for the GPU to present in the stats
			CTimer timer;
			m_spSwapChain->Present(m_nSwapInterval, flags);

			// Update GPU timing stats
			CApplication::Statistics_t& stats = CApplication::Get().GetStatistics();
			stats.m_flGPURenderTime += timer.ElapsedMillis();
		}
	}

	void CGraphicsContext::UpdateGPUMemoryInfo()
	{
		if (!m_bGpuMemoryQuerySupported || m_spDXGIAdapter3 == nullptr)
			return;

		if (m_nQPCFreq != 0) {
			LARGE_INTEGER now{};
			QueryPerformanceCounter(&now);

			const uint64_t nowQpc = static_cast<uint64_t>(now.QuadPart);
			const uint64_t intervalQpc = m_nQPCFreq / kQPCIntervalDivisor;

			if (m_nLastGPUMemQueryQPC != 0 && (nowQpc - m_nLastGPUMemQueryQPC) < intervalQpc)
				return;

			m_nLastGPUMemQueryQPC = nowQpc;
		}

		DXGI_QUERY_VIDEO_MEMORY_INFO local{};
		if (SUCCEEDED(m_spDXGIAdapter3->QueryVideoMemoryInfo(
			0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &local))) {
			m_GPUMemoryInfo.m_nLocalUsedBytes = local.CurrentUsage;
			m_GPUMemoryInfo.m_nLocalBudgetBytes = local.Budget;
		}

		DXGI_QUERY_VIDEO_MEMORY_INFO nonLocal{};
		if (SUCCEEDED(m_spDXGIAdapter3->QueryVideoMemoryInfo(
			0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &nonLocal))) {
			m_GPUMemoryInfo.m_nNonLocalUsedBytes = nonLocal.CurrentUsage;
			m_GPUMemoryInfo.m_nNonLocalBudgetBytes = nonLocal.Budget;
		}
	}
}