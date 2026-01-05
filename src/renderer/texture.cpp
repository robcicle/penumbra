#include "ppch.h"
#include "renderer/texture.h"

#include "core/application.h"
#include "renderer/renderer.h"

namespace penumbra
{
	namespace Utils
	{
		// Converts ImageFormat to a DXGI_FORMAT
		static DXGI_FORMAT ToDXGIFormat(ImageFormat& format)
		{
			switch (format)
			{
			case ImageFormat::None:		PENUMBRA_CORE_ASSERT(false, "D3D11Texture: ImageFormat::None is not supported!"); return DXGI_FORMAT_UNKNOWN;
			case ImageFormat::R8:		return DXGI_FORMAT_R8_UNORM;
			case ImageFormat::RG8:		return DXGI_FORMAT_R8G8_UNORM;
			case ImageFormat::RGBA8:	return DXGI_FORMAT_R8G8B8A8_UNORM;
			case ImageFormat::RGBA16F:	return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case ImageFormat::RGB32F:	return DXGI_FORMAT_R32G32B32_FLOAT;
			case ImageFormat::RGBA32F:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			PENUMBRA_CORE_ASSERT(false, "Unknown ImageFormat!");
			return DXGI_FORMAT_UNKNOWN;
		}

		// Gets the number of channels for a given ImageFormat
		static uint32_t GetChannelCount(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::R8:		return kChannelR;
			case ImageFormat::RG8:		return kChannelRG;
			case ImageFormat::RGBA8:    return kChannelRGBA;
			case ImageFormat::RGBA16F:	return kChannelRGBA;
			case ImageFormat::RGB32F:	return kChannelRGB;
			case ImageFormat::RGBA32F:  return kChannelRGBA;
			}
			return 0;
		}
	}

	// Returns the bytes per pixel for a given ImageFormat
	uint32_t CTexture::GetBPP(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::R8:		return kBPPR8;
		case ImageFormat::RG8:		return kBPPRG8;
		case ImageFormat::RGBA8:    return kBPPRGBA8;
		case ImageFormat::RGBA16F:	return kBPPRGBA16F;
		case ImageFormat::RGB32F:	return kBPPRGB32F;
		case ImageFormat::RGBA32F:  return kBPPRGBA32F;
		}
		PENUMBRA_CORE_ASSERT(false, "Unknown ImageFormat!");
		return 0;
	}

	uint32_t CTexture::CalculateMipMapCount(uint32_t width, uint32_t height)
	{
		// Calculate number of mip levels based off of the largest dimension
		uint32_t levels = 1;
		uint32_t size = std::max(width, height);
		// Shift right until size is 1
		while (size > 1) {
			size >>= 1;
			levels++;
		}

		return levels;
	}

	CTexture2D::CTexture2D(const TextureSpecification_t& specification, Buffer_t data)
		: m_Path(""), m_Specification(specification)
	{
		PENUMBRA_PROFILE_FUNC();

		// Resolve the DXGI format
		const DXGI_FORMAT format = Utils::ToDXGIFormat(m_Specification.m_Format);


		CGraphicsContext* pContext = CApplication::Get().GetWindow().GetGraphicsContext();
		ComPtr<ID3D11Device> spDevice = pContext->GetDevice();
		m_spContext = pContext->GetContext();
		// Ensure we have a valid device context
		PENUMBRA_CORE_ASSERT(m_spContext, "Texture2D: Could not get ID3D11DeviceContext from D3D11Context!");

		// Create the texture description
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = m_Specification.m_nWidth;		// Texture width
		textureDesc.Height = m_Specification.m_nHeight;		// Texture height
		textureDesc.MipLevels = kTextureMipLevels;			// No mipmaps for now
		textureDesc.ArraySize = kTextureArraySize;			// Single texture
		textureDesc.Format = format;						// Texture format
		textureDesc.SampleDesc.Count = kTextureSampleCount;	// No multisampling
		textureDesc.Usage = D3D11_USAGE_DEFAULT;			// Default usage
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;	// Bind as shader resource
		textureDesc.CPUAccessFlags = kTextureCPUAccessFlags;	// No CPU access
		textureDesc.MiscFlags = kTextureMiscFlags;			// No misc flags

		CRenderer::Submit([this, spDevice, textureDesc]() mutable
			{
				// Create the texture
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateTexture2D");
				HRESULT hr = spDevice->CreateTexture2D(&textureDesc, nullptr, m_spTexture.GetAddressOf());
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Texture2D: Failed to create Texture2D.");
			});

		// Create the shader resource view description
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = format;	// Same format as texture
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;	// 2D texture
		srvDesc.Texture2D.MipLevels = kTextureMipLevels;		// No mipmaps

		CRenderer::Submit([this, spDevice, srvDesc]() mutable
			{
				// Create the shader resource view
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateShaderResourceView");
				HRESULT hr = spDevice->CreateShaderResourceView(m_spTexture.Get(), &srvDesc, m_spShaderResourceView.GetAddressOf());
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Texture2D: Failed to create ShaderResourceView.");
				// If creation failed, reset the texture pointer
				if (FAILED(hr)) {
					m_spTexture.Reset();
				}
			});

		// If initial data is provided, set it
		if (data) {
			SetData(data);
		}
	}

	void CTexture2D::SetData(Buffer_t data)
	{
		PENUMBRA_PROFILE_FUNC();

		// Validate data
		if (!data.m_pData || data.m_nSize == 0) {
			PENUMBRA_CORE_WARN("Texture2D: No data provided to SetData!");
			return;
		}

		// Calculate expected data size
        const uint32_t bytesPerPixel = CTexture::GetBPP(m_Specification.m_Format);
        const uint32_t expectedSize = m_Specification.m_nWidth * m_Specification.m_nHeight * bytesPerPixel;
		// Ensure data size matches expected size
		PENUMBRA_CORE_ASSERT(data.m_nSize == expectedSize, "Texture2D: Data size mismatch!");

		// Define the region to update
		D3D11_BOX box = {};
		box.left	= kBoxLeft;
		box.top		= kBoxTop;
		box.front	= kBoxFront;
		box.right	= m_Specification.m_nWidth;		// Texture width
		box.bottom	= m_Specification.m_nHeight;	// Texture height
		box.back	= kBoxBack;

		// Copy data into a vector
		std::vector<uint8_t> dataBuffer = std::vector<uint8_t>(
			static_cast<const uint8_t*>(data.m_pData),
			static_cast<const uint8_t*>(data.m_pData) + data.m_nSize
		);

		// Calculate row pitch (number of bytes per row)
		const uint32_t rowPitch = m_Specification.m_nWidth * bytesPerPixel;
		//PENUMBRA_CORE_ASSERT((rowPitch % 4) == 0, "Texture2D: Row pitch must be a multiple of 4 bytes for D3D11.");

		// std::move dataBuffer into the lambda to avoid copying
		CRenderer::Submit([this, box, dataBuffer = std::move(dataBuffer), rowPitch]() mutable
			{
				// Update the texture with the new data
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::UpdateSubresource");
				m_spContext->UpdateSubresource(
					m_spTexture.Get(),	// Destination texture
					0, &box,			// Destination subresource and box
					dataBuffer.data(),	// Source data
					rowPitch, 0			// Source row pitch and depth pitch
				);
			});

		// Mark texture as loaded
		m_bIsLoaded = true;
	}

	void CTexture2D::Bind(uint32_t nSlot) const
	{
		PENUMBRA_PROFILE_FUNC();

		CRenderer::Submit([this, nSlot]()
			{
				// Ensure the texture is loaded
				if (m_spShaderResourceView == nullptr) {
					PENUMBRA_CORE_WARN("Texture2D: Trying to bind a texture that is not loaded!");
					return;
				}

				// Bind the shader resource view to the pixel shader stage
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::PSSetShaderResources");
				m_spContext->PSSetShaderResources(nSlot, 1, m_spShaderResourceView.GetAddressOf());
			});
	}
}