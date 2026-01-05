#include "ppch.h"
#include "renderer/structured_buffer.h"

#include "core/application.h"
#include "renderer/renderer.h"

namespace penumbra
{
    CStructuredBuffer::CStructuredBuffer(const StructuredBufferSpecification_t& spec)
        : m_Spec(spec)
    {
        PENUMBRA_PROFILE_FUNC();

        // Get D3D11 device/context
        CGraphicsContext* pContext = CApplication::Get().GetWindow().GetGraphicsContext();
        ComPtr<ID3D11Device> spDevice = pContext->GetDevice();
        m_spContext = pContext->GetContext();
        PENUMBRA_CORE_ASSERT(spDevice, "StructuredBuffer: No D3D11 device!");
        PENUMBRA_CORE_ASSERT(m_spContext, "StructuredBuffer: No D3D11 context!");

        uint32_t nByteWidth = m_Spec.m_nElementSize * m_Spec.m_nElementCount;

        // Describe the buffer
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = nByteWidth;
        desc.StructureByteStride = m_Spec.m_nElementSize;
        desc.MiscFlags = 0;

        if (m_Spec.m_bAllowUAV)
        {
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
            desc.CPUAccessFlags = 0;

            if (m_Spec.m_bAllowRawUAV)
            {
                desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
                desc.StructureByteStride = 0; // RAW buffers do not use a stride
            }
            else
            {
                desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
                desc.StructureByteStride = m_Spec.m_nElementSize;
            }
        }
        else
        {
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = m_Spec.m_bAllowRawUAV ? D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS
                : D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        }

        // Create the actual buffer
        CRenderer::Submit([this, spDevice, desc]()
            {
                PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateBuffer");
                HRESULT hr = spDevice->CreateBuffer(&desc, nullptr, m_spBuffer.GetAddressOf());
                PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "StructuredBuffer: Failed to create buffer!");
            });

        // Create our SRV
        if (!m_Spec.m_bAllowRawUAV) // Only create the SRV for structured buffers or non-RAW buffers
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            srvDesc.Buffer.FirstElement = 0;
            srvDesc.Buffer.NumElements = m_Spec.m_nElementCount;

            CRenderer::Submit([this, spDevice, srvDesc]()
                {
                    PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateShaderResourceView");
                    HRESULT hr = spDevice->CreateShaderResourceView(m_spBuffer.Get(), &srvDesc, m_spSRV.GetAddressOf());
                    PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "StructuredBuffer: Failed to create SRV!");
                });
        }
        else
        {
            // RAW SRV
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
            srvDesc.BufferEx.FirstElement = 0;
            srvDesc.BufferEx.NumElements = m_Spec.m_nElementCount;
            srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;

            CRenderer::Submit([this, spDevice, srvDesc]()
                {
                    PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateShaderResourceView (RAW)");
                    HRESULT hr = spDevice->CreateShaderResourceView(m_spBuffer.Get(), &srvDesc, m_spSRV.GetAddressOf());
                    PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "StructuredBuffer: Failed to create RAW SRV!");
                });
        }

        // Create UAV if it's requested
        if (m_Spec.m_bAllowUAV)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement = 0;
            uavDesc.Buffer.NumElements = m_Spec.m_nElementCount;

            if (m_Spec.m_bAllowRawUAV)
            {
                uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;        // RAW buffers require TYPELESS
                uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
            }
            else
            {
                uavDesc.Format = DXGI_FORMAT_UNKNOWN;              // Structured Buffer
                uavDesc.Buffer.Flags = m_Spec.m_bUseCounter ? D3D11_BUFFER_UAV_FLAG_COUNTER : 0;
            }

            CRenderer::Submit([this, spDevice, uavDesc]()
                {
                    HRESULT hr = spDevice->CreateUnorderedAccessView(m_spBuffer.Get(), &uavDesc, m_spUAV.GetAddressOf());
                    PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "StructuredBuffer: Failed to create UAV!");
                });
        }
    }

    void CStructuredBuffer::BindSRV(ShaderType stage)
    {
        PENUMBRA_PROFILE_FUNC();

        CRenderer::Submit([this, stage]()
            {
                const uint32_t nBinding = m_Spec.m_nSRVBinding;

                ShaderType stageToBind = stage == ShaderType::SHADER_TYPE_NONE ? m_Spec.m_ShaderType : stage;

                // Bind the SRV to the required shader stage
                switch (stageToBind)
                {
                case ShaderType::SHADER_TYPE_VERTEX:
                {
                    PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::VSSetShaderResources");
                    m_spContext->VSSetShaderResources(nBinding, 1, m_spSRV.GetAddressOf());
                    break;
                }
                case ShaderType::SHADER_TYPE_PIXEL:
                {
                    PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::PSSetShaderResources");
                    m_spContext->PSSetShaderResources(nBinding, 1, m_spSRV.GetAddressOf());
                    break;
                }
                case ShaderType::SHADER_TYPE_GEOMETRY:
                {
                    PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::GSSetShaderResources");
                    m_spContext->GSSetShaderResources(nBinding, 1, m_spSRV.GetAddressOf());
                    break;
                }
                case ShaderType::SHADER_TYPE_COMPUTE:
                {
                    PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::CSSetShaderResources");
                    m_spContext->CSSetShaderResources(nBinding, 1, m_spSRV.GetAddressOf());
                    break;
                }
                default:
                    PENUMBRA_CORE_ASSERT(false, "StructuredBuffer: Invalid shader type!");
                    break;
                }
            });
    }

    void CStructuredBuffer::UnbindSRV(ShaderType stage)
    {
        PENUMBRA_PROFILE_FUNC();

        CRenderer::Submit([this, stage]()
            {
                const uint32_t nBinding = m_Spec.m_nSRVBinding;

                ShaderType stageToBind = stage == ShaderType::SHADER_TYPE_NONE ? m_Spec.m_ShaderType : stage;

                ID3D11ShaderResourceView* nullSRV = nullptr;

                // Bind the SRV to the required shader stage
                switch (stageToBind)
                {
                case ShaderType::SHADER_TYPE_VERTEX:
                {
                    PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::VSSetShaderResources");
                    m_spContext->VSSetShaderResources(nBinding, 1, &nullSRV);
                    break;
                }
                case ShaderType::SHADER_TYPE_PIXEL:
                {
                    PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::PSSetShaderResources");
                    m_spContext->PSSetShaderResources(nBinding, 1, &nullSRV);
                    break;
                }
                case ShaderType::SHADER_TYPE_GEOMETRY:
                {
                    PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::GSSetShaderResources");
                    m_spContext->GSSetShaderResources(nBinding, 1, &nullSRV);
                    break;
                }
                case ShaderType::SHADER_TYPE_COMPUTE:
                {
                    PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::CSSetShaderResources");
                    m_spContext->CSSetShaderResources(nBinding, 1, &nullSRV);
                    break;
                }
                default:
                    PENUMBRA_CORE_ASSERT(false, "StructuredBuffer: Invalid shader type!");
                    break;
                }
            });
    }

    void CStructuredBuffer::BindUAV()
    {
        PENUMBRA_PROFILE_FUNC();
        PENUMBRA_CORE_ASSERT(m_Spec.m_bAllowUAV, "CStructuredBuffer: UAV not created!");

        CRenderer::Submit([this]()
            {
                ID3D11ShaderResourceView* nullSRV = nullptr;
                const UINT srvSlot = m_Spec.m_nSRVBinding;
                const UINT uavSlot = m_Spec.m_nUAVBinding;

                // Unbind our SRVs for all states
                if (m_spSRV)
                {
                    m_spContext->VSSetShaderResources(srvSlot, 1, &nullSRV);
                    m_spContext->PSSetShaderResources(srvSlot, 1, &nullSRV);
                    m_spContext->GSSetShaderResources(srvSlot, 1, &nullSRV);
                    m_spContext->CSSetShaderResources(srvSlot, 1, &nullSRV);
                }

				// Bind our UAV to the compute shader
                m_spContext->CSSetUnorderedAccessViews(uavSlot, 1, m_spUAV.GetAddressOf(), nullptr);
            });
    }

    void CStructuredBuffer::UnbindUAV()
    {
        PENUMBRA_PROFILE_FUNC();

		// Unbind the UAV from the compute shader
        CRenderer::Submit([this]()
            {
                PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::CSSetUnorderedAccessViews");
                ID3D11UnorderedAccessView* nullUAV = nullptr;
                m_spContext->CSSetUnorderedAccessViews(m_Spec.m_nUAVBinding, 1, &nullUAV, nullptr);
            });
    }

    void CStructuredBuffer::SetData(const void* pData, uint32_t count)
    {
        PENUMBRA_PROFILE_FUNC();

        // Ensure we don't write more elements than the buffer can hold
        PENUMBRA_CORE_ASSERT(count <= m_Spec.m_nElementCount, "StructuredBuffer: Attempt to write more elements than available!");

        // Calculate the size to write
        uint32_t writeSize = count * m_Spec.m_nElementSize;

        // Copy data to preserve lifetime for renderer thread
        std::vector<std::byte> copy(writeSize);
        memcpy(copy.data(), pData, writeSize);  // Copy the data

        CRenderer::Submit([this, copy = std::move(copy)]() mutable
            {
                // Map the buffer for writing
                D3D11_MAPPED_SUBRESOURCE mapped{};
                {
                    PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::Map");
                    HRESULT hr = m_spContext->Map(
                        m_spBuffer.Get(), 0,        // Buffer and subresource
                        D3D11_MAP_WRITE_DISCARD, 0, // Map type and flags
                        &mapped                     // Mapped resource
                    );
                    // Ensure mapping succeeded
                    PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "StructuredBuffer: Failed to map!");
                }

                // Copy the data into the mapped buffer
                memcpy(mapped.pData, copy.data(), copy.size());
                {
                    // Unmap the buffer
                    PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::Unmap");
                    m_spContext->Unmap(m_spBuffer.Get(), 0);
                }
            });
    }

    void CStructuredBuffer::ResetUAVCounter(uint32_t value)
    {
        PENUMBRA_PROFILE_FUNC();
        PENUMBRA_CORE_ASSERT(m_Spec.m_bAllowUAV, "CStructuredBuffer: UAV not created!");
        PENUMBRA_CORE_ASSERT(m_Spec.m_bUseCounter, "CStructuredBuffer: UAV counter not enabled!");

		// D3D11 requires that a resource is not bound as SRV (through input) and UAV (through output)
        // at the same time between any of the shader stages. To safetly reset the counter we need to,
        // unbind the SRV for this buffer from all ShaderStages, unbind the UAV from our CS Stage and then
		// re-bind the UAV to our CS stage with an initial counter value.
        CRenderer::Submit([this, value]()
            {
                ID3D11ShaderResourceView* nullSRV = nullptr;
                ID3D11UnorderedAccessView* nullUAV = nullptr;

                const UINT srvSlot = m_Spec.m_nSRVBinding;
                const UINT uavSlot = m_Spec.m_nUAVBinding;

                // Unbind SRV from ALL shader stages at this slot
                if (m_spSRV)
                {
                    m_spContext->VSSetShaderResources(srvSlot, 1, &nullSRV);
                    m_spContext->PSSetShaderResources(srvSlot, 1, &nullSRV);
                    m_spContext->GSSetShaderResources(srvSlot, 1, &nullSRV);
                    m_spContext->CSSetShaderResources(srvSlot, 1, &nullSRV);
                }

                // Unbind UAV from CS at this slot
                m_spContext->CSSetUnorderedAccessViews(uavSlot, 1, &nullUAV, nullptr);

                // Re-bind UAV with counter reset based on value
                m_spContext->CSSetUnorderedAccessViews(uavSlot, 1, m_spUAV.GetAddressOf(), &value);
            });
    }
}
