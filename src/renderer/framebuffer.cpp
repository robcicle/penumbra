#include "ppch.h"
#include "renderer/framebuffer.h"

#include "core/application.h"
#include "renderer/renderer.h"

namespace penumbra
{
	namespace Utils
	{
		static const DXGI_FORMAT ToDXGIFormat(const FramebufferTextureFormat& format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::R8:               return DXGI_FORMAT_R8_UNORM;
			case FramebufferTextureFormat::RGBA8:            return DXGI_FORMAT_R8G8B8A8_UNORM;
			case FramebufferTextureFormat::RGBA16F:          return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case FramebufferTextureFormat::RGBA32F:          return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case FramebufferTextureFormat::RED_INTEGER:      return DXGI_FORMAT_R32_SINT;
			case FramebufferTextureFormat::DEPTH24STENCIL8:  return DXGI_FORMAT_R24G8_TYPELESS;
			default:
				PENUMBRA_CORE_ASSERT(false, "Unknown FramebufferTextureFormat!");
				return DXGI_FORMAT_UNKNOWN;
			}
		}

		static const uint32_t GetFormatBPP(const FramebufferTextureFormat& format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::R8:               return 1;
			case FramebufferTextureFormat::RGBA8:            return 4;
			case FramebufferTextureFormat::RGBA16F:          return 8;
			case FramebufferTextureFormat::RGBA32F:          return 16;
			case FramebufferTextureFormat::RED_INTEGER:      return 4;
			case FramebufferTextureFormat::DEPTH24STENCIL8:  return 4;
			default:
				PENUMBRA_CORE_ASSERT(false, "Unknown FramebufferTextureFormat!");
				return 0;
			}
		}

		static const bool IsDepth(const FramebufferTextureFormat& format)
		{
			return format == FramebufferTextureFormat::DEPTH24STENCIL8;
		}

		static void CreateColorAttachment(
			ComPtr<ID3D11Device>& spDevice, int nSamples, DXGI_FORMAT format,
			uint32_t nWidth, uint32_t nHeight,
			ComPtr<ID3D11Texture2D>& spOutTex,
			ComPtr<ID3D11RenderTargetView>& spOutRTV,
			ComPtr<ID3D11ShaderResourceView>& spOutSRV)
		{
			PENUMBRA_PROFILE_FUNC();

			HRESULT hr = S_OK;
			const bool bMultisampled = nSamples > 1;

			// Define the texture description
			D3D11_TEXTURE2D_DESC texDesc{};
			texDesc.Width = nWidth;								// Texture width (screen size)
			texDesc.Height = nHeight;							// Texture height (screen size)
			texDesc.MipLevels = kDefaultFramebufferMipLevels;	// No mipmaps
			texDesc.ArraySize = kDefaultFramebufferArraySize;	// Single texture
			texDesc.Format = format;				// Texture format (DXGI)
			texDesc.SampleDesc.Count = nSamples;	// Multisampling count
			texDesc.Usage = D3D11_USAGE_DEFAULT;	// GPU read/write access
			texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;	// Bind as RTV and SRV
			// Create the texture
			{
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateTexture2D");
				hr = spDevice->CreateTexture2D(&texDesc, nullptr, spOutTex.GetAddressOf());
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Failed to create color texture!");
			}

			// Create the Render Target View (RTV)
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
			rtvDesc.Format = format;	// RTV format
			rtvDesc.ViewDimension = bMultisampled ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;	// View dimension based on multisampling
			rtvDesc.Texture2D.MipSlice = kDefaultFramebufferMipSlice;	// Use the first mip level
			// Create the RTV
			{
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateRenderTargetView");
				hr = spDevice->CreateRenderTargetView(spOutTex.Get(), &rtvDesc, spOutRTV.GetAddressOf());
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Failed to create RTV!");
			}

			// Create the Shader Resource View (SRV)
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = format;	// SRV format
			srvDesc.ViewDimension = bMultisampled ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;	// View dimension based on multisampling
			srvDesc.Texture2D.MostDetailedMip = kDefaultFramebufferMostDetailedMip;	// Most detailed mip level
			srvDesc.Texture2D.MipLevels = kDefaultFramebufferMipLevels;				// Number of mip levels
			// Create the SRV
			{
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateShaderResourceView");
				hr = spDevice->CreateShaderResourceView(spOutTex.Get(), &srvDesc, spOutSRV.GetAddressOf());
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Failed to create SRV!");
			}
		}

		static void CreateDepthAttachment(
			ComPtr<ID3D11Device>& spDevice, int nSamples, FramebufferTextureFormat format,
			const uint32_t nWidth, const uint32_t nHeight,
			ComPtr<ID3D11Texture2D>& spOutTex,
			ComPtr<ID3D11DepthStencilView>& spOutDSV,
			ComPtr<ID3D11ShaderResourceView>& spOutSRV)
		{
			HRESULT hr = S_OK;

			// Create the depth texture
			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = nWidth;	// Texture width (screen size)
			desc.Height = nHeight;	// Texture height (screen size)
			desc.MipLevels = kDefaultFramebufferMipLevels;		// No mipmaps
			desc.ArraySize = kDefaultFramebufferArraySize;		// Single texture
			desc.Format = Utils::ToDXGIFormat(format);			// Texture format (DXGI)
			desc.SampleDesc.Count = nSamples;	// Multisampling count
			desc.Usage = D3D11_USAGE_DEFAULT;	// GPU read/write access
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;	// Bind as DSV and SRV
			// Create the texture
			{
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateTexture2D");
				hr = spDevice->CreateTexture2D(&desc, nullptr, spOutTex.GetAddressOf());
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Depth texture creation failed!");
			}

			// Create the Depth Stencil View (DSV)
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
			dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;		// DSV format
			dsvDesc.ViewDimension = desc.SampleDesc.Count > 1	// Multisampled or not
				? D3D11_DSV_DIMENSION_TEXTURE2DMS	// Multisampled
				: D3D11_DSV_DIMENSION_TEXTURE2D;	// Not multisampled
			// Create the DSV
			{
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateDepthStencilView");
				hr = spDevice->CreateDepthStencilView(spOutTex.Get(), &dsvDesc, spOutDSV.GetAddressOf());
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "DSV creation failed!");
			}

			// Create the Shader Resource View (SRV)
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;	// SRV format
			srvDesc.ViewDimension = desc.SampleDesc.Count > 1	// Multisampled or not
				? D3D11_SRV_DIMENSION_TEXTURE2DMS	// Multisampled
				: D3D11_SRV_DIMENSION_TEXTURE2D;	// Not multisampled
			srvDesc.Texture2D.MipLevels = kDefaultFramebufferMipLevels;	// Number of mip levels
			// Create the SRV
			{
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateShaderResourceView");
				hr = spDevice->CreateShaderResourceView(spOutTex.Get(), &srvDesc, spOutSRV.GetAddressOf());
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Depth SRV creation failed!");
			}
		}

		static void CopyResourceToView(
			const ComPtr<ID3D11DeviceContext>& spContext,
			ComPtr<ID3D11View> spView,
			ComPtr<ID3D11Resource> spSrc)
		{
			// Copy resource data from src to the resource bound to the view
			if (!spView || !spSrc)
				return;

			// Get the resource from the view
			ComPtr<ID3D11Resource> dst = nullptr;
			{
				PENUMBRA_PROFILE_SCOPE("ID3D11View::GetResource");
				spView->GetResource(dst.GetAddressOf());
			}
			if (dst != nullptr) {
				// Copy the resource
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::CopyResource");
				spContext->CopyResource(dst.Get(), spSrc.Get());
			}
		}
	}

	CFramebuffer::CFramebuffer(const FramebufferSpecification_t& spec)
		: m_Specification(spec)
	{
		PENUMBRA_PROFILE_FUNC();

		// Get D3D11 device and context from the application window
		CGraphicsContext* pContext = CApplication::Get().GetWindow().GetGraphicsContext();
		m_spDevice = pContext->GetDevice();
		m_spContext = pContext->GetContext();

		Invalidate();
	}

	void CFramebuffer::Invalidate()
	{
		PENUMBRA_PROFILE_FUNC();

		// Release existing resources
		ReleaseResources();

		const bool bMultisampled = m_Specification.m_nSamples > 1;

		// Create color and depth attachments based on the specification
		for (const auto& att : m_Specification.m_Attachments.m_vAttachments) {
			// Color attachment
			if (!Utils::IsDepth(att.m_TextureFormat)) {
				CRenderer::Submit([this, &att, bMultisampled]()
					{
						// Create color attachment
						ComPtr<ID3D11Texture2D> spTex			= nullptr;
						ComPtr<ID3D11RenderTargetView> spRTV	= nullptr;
						ComPtr<ID3D11ShaderResourceView> spSRV	= nullptr;
						Utils::CreateColorAttachment(
							m_spDevice,
							m_Specification.m_nSamples,
							Utils::ToDXGIFormat(att.m_TextureFormat),
							m_Specification.m_nWidth,
							m_Specification.m_nHeight,
							spTex, spRTV, spSRV
						);
						// Store references
						m_vspTextures.push_back(spTex);
						m_vspRTVs.push_back(spRTV);
						m_vspSRVs.push_back(spSRV);
					});
			}
			// Depth attachment
			else {
				CRenderer::Submit([this, &att]()
					{
						// Create depth attachment
						Utils::CreateDepthAttachment(
							m_spDevice,
							m_Specification.m_nSamples,
							att.m_TextureFormat,
							m_Specification.m_nWidth,
							m_Specification.m_nHeight,
							m_spDepthTexture,
							m_spDepthDSV,
							m_spDepthSRV
						);
					});
			}
		}
	}

	void CFramebuffer::ReleaseResources()
	{
		PENUMBRA_PROFILE_FUNC();

		CRenderer::Submit([this]()
			{
				// Release all resources
				m_vspTextures.clear();
				m_vspRTVs.clear();
				m_vspSRVs.clear();
				m_spDepthTexture.Reset();
				m_spDepthSRV.Reset();
				m_spDepthDSV.Reset();
			});
	}

	void CFramebuffer::Bind() const
	{
		PENUMBRA_PROFILE_FUNC();

		// Bind the framebuffer's render targets and depth stencil view
		CRenderer::Submit([this]()
			{
				// Prepare RTVs
				std::vector<ID3D11RenderTargetView*> vpRTVs;
				vpRTVs.reserve(m_vspRTVs.size());
				// Collect RTV pointers
				for (auto& cptr : m_vspRTVs) vpRTVs.push_back(cptr.Get());

				{
					// Bind RTVs and DSV
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::OMSetRenderTargets");
					m_spContext->OMSetRenderTargets(
						static_cast<UINT>(vpRTVs.size()),
						vpRTVs.data(), m_spDepthDSV.Get());
				}

				// Set viewport
				D3D11_VIEWPORT viewport = {};
				viewport.TopLeftX = 0;
				viewport.TopLeftY = 0;
				viewport.Width = m_Specification.m_nWidth;
				viewport.Height = m_Specification.m_nHeight;
				viewport.MinDepth = kMinDepth;
				viewport.MaxDepth = kMaxDepth;
				{
					// Set the viewport
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::RSSetViewports");
					m_spContext->RSSetViewports(1, &viewport);
				}
			});
	}

	void CFramebuffer::Unbind() const
	{
		PENUMBRA_PROFILE_FUNC();

		// Restore default backbuffer or reset bindings
		CRenderCommand::Bind();

		// If this framebuffer is the swap chain target, copy its contents back to the swap chain buffers
		if (!m_Specification.m_bSwapChainTarget)
			return;

		CRenderer::Submit([this]()
			{
				// Get currently bound render targets
				ComPtr<ID3D11RenderTargetView> spBoundRTV = nullptr;
				ComPtr<ID3D11DepthStencilView> spBoundDSV = nullptr;

				{
					// Retrieve currently bound RTV and DSV
					PENUMBRA_PROFILE_SCOPE("OMGetRenderTargets");
					m_spContext->OMGetRenderTargets(1, &spBoundRTV, &spBoundDSV);
				}

				// Copy color attachment to back buffer
				if (m_vspSRVs.size() > 0 && spBoundRTV) {
					Utils::CopyResourceToView(m_spContext, spBoundRTV, m_vspTextures[0]);
				}

				// Copy depth attachment to back buffers depth buffer
				if (m_spDepthTexture != nullptr && spBoundDSV) {
					Utils::CopyResourceToView(m_spContext, spBoundDSV, m_spDepthTexture);
				}
			});
	}

	void CFramebuffer::Resize(uint32_t nWidth, uint32_t nHeight)
	{
		PENUMBRA_PROFILE_FUNC();

		// Check for valid dimensions
		if (nWidth == 0 || nHeight == 0 || nWidth > kMaxFramebufferSize || nHeight > kMaxFramebufferSize) {
			PENUMBRA_CORE_WARN("D3D11Framebuffer: Attempted to rezize framebuffer to {0}, {1}", nWidth, nHeight);
			return;
		}

		// Needs to be submitted so it is done on the render thread
		CRenderer::Submit([this, nWidth, nHeight]()
			{
				m_Specification.m_nWidth = nWidth;
				m_Specification.m_nHeight = nHeight;
			});

		Invalidate();
	}

	int CFramebuffer::ReadPixel(uint32_t nAttachmentIndex, int nX, int nY) const
	{
		// All our GPU commands (D3D11 functions) should be submitted to a queue, this is the one time where we cannot.
		// This also means the pixel we read would actually be a frame behind from when we expect it to be read.
		// This MUST be account for when you're requesting the pixel data.
		PENUMBRA_PROFILE_FUNC();

		PENUMBRA_CORE_ASSERT(nAttachmentIndex < m_vspSRVs.size(), "Invalid attachment index!");

		HRESULT hr = S_OK;

		const ComPtr<ID3D11Texture2D>& spSrcTexture = m_vspTextures[nAttachmentIndex];
		const FramebufferTextureFormat& format = m_Specification.m_Attachments.m_vAttachments[nAttachmentIndex].m_TextureFormat;

		// Get the texture description
		D3D11_TEXTURE2D_DESC desc{};
		spSrcTexture->GetDesc(&desc);

		// Create a staging texture for CPU readback
		ComPtr<ID3D11Texture2D> spStaging;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		// Create the staging texture
		{
			PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateTexture2D");
			hr = m_spDevice->CreateTexture2D(&desc, nullptr, spStaging.GetAddressOf());
			PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Failed to create staging texture!");
		}

		// Copy GPU texture to CPU-readable texture
		{
			PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::CopyResource");
			m_spContext->CopyResource(spStaging.Get(), spSrcTexture.Get());
		}

		// Map and read pixel
		D3D11_MAPPED_SUBRESOURCE mapped{};
		{
			PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::Map");
			hr = m_spContext->Map(spStaging.Get(), 0, D3D11_MAP_READ, 0, &mapped);
			PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Failed to map staging texture!");
		}

		// Calculate pixel position and read value
		int* pixelData = (int*)mapped.pData;
		int rowPitch = mapped.RowPitch / sizeof(int);
		int pixelValue = pixelData[nY * rowPitch + nX];

		// Unmap the staging texture
		{
			PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::Unmap");
			m_spContext->Unmap(spStaging.Get(), 0);
		}

		return pixelValue;
	}

	void CFramebuffer::BindTexture(uint32_t nAttachmentIndex, uint32_t nSlot) const
	{
		PENUMBRA_PROFILE_FUNC();

		// Validate attachment index
		if (nAttachmentIndex >= m_vspSRVs.size())
			return;

		CRenderer::Submit([this, nSlot, nAttachmentIndex]()
			{
				// Bind the shader resource view to the specified slot
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::PSSetShaderResources");
				m_spContext->PSSetShaderResources(nSlot, 1, m_vspSRVs[nAttachmentIndex].GetAddressOf());
			});
	}

	void CFramebuffer::BindDepthTexture(uint32_t nSlot, ShaderType stage) const
	{
		PENUMBRA_PROFILE_FUNC();

		// Validate depth SRV
		if (!m_spDepthSRV)
			return;

		CRenderer::Submit([this, nSlot, stage]()
			{
				switch (stage)
				{
				// Bind the depth shader resource view to the specified slot
				case SHADER_TYPE_PIXEL:
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::PSSetShaderResources");
					m_spContext->PSSetShaderResources(nSlot, 1, m_spDepthSRV.GetAddressOf());
					break;
				}
				case SHADER_TYPE_COMPUTE:
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::CSSetShaderResources");
					m_spContext->CSSetShaderResources(nSlot, 1, m_spDepthSRV.GetAddressOf());
					break;
				}
				}
			});
	}

	void CFramebuffer::UnbindTexture(uint32_t nSlot, ShaderType stage) const
	{
		PENUMBRA_PROFILE_FUNC();

		CRenderer::Submit([this, nSlot, stage]()
			{
				// Unbind any shader resource view from the specified slot
				ID3D11ShaderResourceView* s = nullptr;
				switch (stage)
				{
				case SHADER_TYPE_PIXEL:
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::PSSetShaderResources");
					m_spContext->PSSetShaderResources(nSlot, 1, &s);
				}
				case SHADER_TYPE_COMPUTE:
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::CSSetShaderResources");
					m_spContext->CSSetShaderResources(nSlot, 1, &s);
				}
				}
			});
	}

	void CFramebuffer::ClearAttachment(uint32_t nAttachmentIndex, int nValue) const
	{
		PENUMBRA_PROFILE_FUNC();

		// Check if it's a color or depth attachment
		if (nAttachmentIndex >= m_vspRTVs.size() && !m_spDepthDSV)
			return;

		const FramebufferTextureFormat& format = m_Specification.m_Attachments.m_vAttachments[nAttachmentIndex].m_TextureFormat;
		float flClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		// Determine clear color based on format
		switch (format)
		{
		case FramebufferTextureFormat::RGBA8:
		case FramebufferTextureFormat::RGBA16F:
		case FramebufferTextureFormat::RGBA32F:
			flClearColor[0] = (float)nValue;
			flClearColor[1] = (float)nValue;
			flClearColor[2] = (float)nValue;
			flClearColor[3] = (float)nValue;
			break;
		case FramebufferTextureFormat::R8:
		case FramebufferTextureFormat::RED_INTEGER:
			flClearColor[0] = (float)nValue;
			break;
		}

		CRenderer::Submit([this, nAttachmentIndex, flClearColor]()
			{
				if (nAttachmentIndex < m_vspRTVs.size()) {
					// Clear the specified attachment
					ID3D11RenderTargetView* pRTV = m_vspRTVs[nAttachmentIndex].Get();
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::ClearRenderTargetView");
					m_spContext->ClearRenderTargetView(pRTV, reinterpret_cast<const float*>(&flClearColor));
				}
				else if (m_spDepthDSV) {
					// Clear the depth attachment
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::ClearDepthStencilView");
					m_spContext->ClearDepthStencilView(
						m_spDepthDSV.Get(),
						D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
						kMaxDepth, kDepthStencilClearValue
					);
				}
			});
	}
}