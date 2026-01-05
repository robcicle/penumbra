#include "ppch.h"
#include "renderer/constant_buffer.h"

#include "core/application.h"
#include "renderer/renderer.h"

namespace penumbra 
{
	namespace Utils
	{
		static uint32_t AlignTo16(uint32_t nSize)
		{
			return (nSize + 15) & ~15;
		}
	}

	CConstantBuffer::CConstantBuffer(uint32_t nSize, uint32_t nBinding, ShaderType shaderType)
		: m_nBinding(nBinding), m_ShaderType(shaderType)
	{
		PENUMBRA_PROFILE_FUNC();

		// Get the D3D11 device and context
		CGraphicsContext* pContext = CApplication::Get().GetWindow().GetGraphicsContext();
		ComPtr<ID3D11Device> spDevice = pContext->GetDevice();
		PENUMBRA_CORE_ASSERT(pContext, "ConstantBuffer: Could not get D3D11Context from Application::Window!");
		PENUMBRA_CORE_ASSERT(spDevice, "ConstantBuffer: Could not get ID3D11Device from D3D11Context!");

		// Align size to 16 bytes as required by D3D11
		const uint32_t nAlignedSize = Utils::AlignTo16(nSize);

		// Describe the constant buffer
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.ByteWidth = nAlignedSize;					// Size of the buffer in bytes (aligned)
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;				// Because we will be updating it frequently
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	// Because this is a Constant Buffer
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// Because of D3D11_USAGE_DYNAMIC

		CRenderer::Submit([this, spDevice, bufferDesc]()
			{
				// Create the constant buffer
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateBuffer");
				HRESULT hr = spDevice->CreateBuffer(&bufferDesc, nullptr, m_spConstantBuffer.GetAddressOf());
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "ConstantBuffer: Could not create ConstantBuffer!");
			});

		// Get the device context
		m_spContext = pContext->GetContext();
		PENUMBRA_CORE_ASSERT(m_spContext, "ConstantBuffer: Could not get ID3D11DeviceContext from D3D11Context!");

		// Bind the buffer to the pipeline
		Bind();
	}

	void CConstantBuffer::Bind(ShaderType stage)
	{
		PENUMBRA_PROFILE_FUNC();

		ShaderType stageToBind = stage == ShaderType::SHADER_TYPE_NONE ? m_ShaderType : stage;
		PENUMBRA_CORE_ASSERT(stageToBind != ShaderType::SHADER_TYPE_NONE, "ConstantBuffer: Shader has unknown ShaderType!");

		CRenderer::Submit([this, stageToBind]()
			{
				ID3D11Buffer** ppBufferAddr = m_spConstantBuffer.GetAddressOf();

				// Bind the constant buffer to the appropriate shader stage
				switch (stageToBind)
				{
				case ShaderType::SHADER_TYPE_VERTEX:
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::VSSetConstantBuffers");
					m_spContext->VSSetConstantBuffers(m_nBinding, 1, ppBufferAddr);
					break;
				}
				case ShaderType::SHADER_TYPE_PIXEL:
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::PSSetConstantBuffers");
					m_spContext->PSSetConstantBuffers(m_nBinding, 1, ppBufferAddr);
					break;
				}
				case ShaderType::SHADER_TYPE_GEOMETRY:
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::GSSetConstantBuffers");
					m_spContext->GSSetConstantBuffers(m_nBinding, 1, ppBufferAddr);
					break;
				}
				case ShaderType::SHADER_TYPE_COMPUTE:
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::CSSetConstantBuffers");
					m_spContext->CSSetConstantBuffers(m_nBinding, 1, ppBufferAddr);
					break;
				}
				case ShaderType::SHADER_TYPE_NONE:
				default:
					PENUMBRA_CORE_ASSERT(false, "ConstantBuffer: Shader has unknown ShaderType!");
					break;
				}
			});
	}

	void CConstantBuffer::SetData(const void* pData, uint32_t nSize, uint32_t nOffset)
	{
		PENUMBRA_PROFILE_FUNC();

		// We only want to validate this in builds where asserts are enabled
#ifdef PENUMBRA_ENABLE_ASSERTS
		// Ensure we don't write outside the bounds of the buffer
		CRenderer::Submit([this, nOffset, nSize]()
			{
				uint32_t nBufferSize = 0;
				{
					// Get the size of the constant buffer
					D3D11_BUFFER_DESC desc;
					m_spConstantBuffer->GetDesc(&desc);
					nBufferSize = desc.ByteWidth;
				}
				PENUMBRA_CORE_ASSERT(nOffset + nSize <= nBufferSize,
					"ConstantBuffer: Trying to set data outside of the bounds of the buffer");
			});
#endif

		// Copy the data to a temporary buffer to ensure it remains valid until the render command is executed
		std::vector<std::byte> vCopy(nSize);
		memcpy(vCopy.data(), pData, nSize);

		CRenderer::Submit([this, copy = std::move(vCopy), nOffset]() mutable
			{
				// Map the constant buffer and update its contents
				D3D11_MAPPED_SUBRESOURCE mappedResource{};
				HRESULT hr = S_OK;
				{
					// Map the constant buffer
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::Map");
					hr = m_spContext->Map(
						m_spConstantBuffer.Get(),	// Constant buffer to map
						0,							// Subresource index (0 for non-texture resources)
						D3D11_MAP_WRITE_DISCARD,	// Map type
						0,							// Map flags
						&mappedResource				// Mapped subresource structure to receive the data pointer
					);
				}
				// Check if mapping was successful
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "ConstantBuffer: Failed to map constant buffer!");

				// Copy the data into the mapped resource at the specified offset
				memcpy(static_cast<uint8_t*>(mappedResource.pData) + nOffset, copy.data(), copy.size());
				{
					// Unmap the constant buffer
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::Unmap");
					m_spContext->Unmap(m_spConstantBuffer.Get(), 0);
				}
			});
	}
}
