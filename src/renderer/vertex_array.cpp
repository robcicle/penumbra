#include "ppch.h"
#include "renderer/vertex_array.h"

#include "core/application.h"
#include "renderer/renderer.h"

namespace penumbra
{
	namespace Utils
	{
		static DXGI_FORMAT ToDXGIFormat(ShaderDataType type)
		{
			switch (type)
			{
			case ShaderDataType::Float:     return DXGI_FORMAT_R32_FLOAT;
			case ShaderDataType::Float2:    return DXGI_FORMAT_R32G32_FLOAT;
			case ShaderDataType::Float3:    return DXGI_FORMAT_R32G32B32_FLOAT;
			case ShaderDataType::Float4:    return DXGI_FORMAT_R32G32B32A32_FLOAT;

			case ShaderDataType::Mat3:      return DXGI_FORMAT_R32G32B32_FLOAT;
			case ShaderDataType::Mat4:      return DXGI_FORMAT_R32G32B32A32_FLOAT;

			case ShaderDataType::Int:       return DXGI_FORMAT_R32_SINT;
			case ShaderDataType::Int2:      return DXGI_FORMAT_R32G32_SINT;
			case ShaderDataType::Int3:      return DXGI_FORMAT_R32G32B32_SINT;
			case ShaderDataType::Int4:      return DXGI_FORMAT_R32G32B32A32_SINT;

			case ShaderDataType::Bool:      return DXGI_FORMAT_R32_UINT;
			}

			PENUMBRA_CORE_ASSERT(false, "Unknown ShaderDataType");
			return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		}

		static void AddInputElementsForLayout(
			std::vector<D3D11_INPUT_ELEMENT_DESC>& vElements,
			const CBufferLayout& layout,
			UINT nSlot,
			bool bInstanced)
		{
			// Iterate over each element in the layout
			for (const auto& e : layout)
			{
				// Determine instanced or vertex data
				UINT nStepRate = bInstanced ? 1 : 0;
				// Determine slot class
				D3D11_INPUT_CLASSIFICATION slotClass = bInstanced ?
					D3D11_INPUT_PER_INSTANCE_DATA :
					D3D11_INPUT_PER_VERTEX_DATA;

				// Handle matrix types specially
				switch (e.m_Type)
				{
				case ShaderDataType::Mat4:
				{
					// Each column of the matrix is a separate input element
					for (UINT i = 0; i < 4; i++)
					{
						D3D11_INPUT_ELEMENT_DESC d{};
						d.SemanticName = e.m_Name.c_str();
						d.SemanticIndex = i;
						d.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
						d.InputSlot = nSlot;
						d.AlignedByteOffset = e.m_nOffset + sizeof(float) * 4 * i;	// Offset for each column
						d.InputSlotClass = slotClass;
						d.InstanceDataStepRate = nStepRate;
						vElements.push_back(d);
					}
					break;
				}
				case ShaderDataType::Mat3:
				{
					// Each column of the matrix is a separate input element
					for (UINT i = 0; i < 3; i++)
					{
						D3D11_INPUT_ELEMENT_DESC d{};
						d.SemanticName = e.m_Name.c_str();
						d.SemanticIndex = i;
						d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
						d.InputSlot = nSlot;
						d.AlignedByteOffset = e.m_nOffset + sizeof(float) * 3 * i;	// Offset for each column
						d.InputSlotClass = slotClass;
						d.InstanceDataStepRate = nStepRate;
						vElements.push_back(d);
					}
					break;
				}
				default:
				{
					// Regular element
					D3D11_INPUT_ELEMENT_DESC d{};
					d.SemanticName = e.m_Name.c_str();
					d.SemanticIndex = 0;
					d.Format = Utils::ToDXGIFormat(e.m_Type);
					d.InputSlot = nSlot;
					d.AlignedByteOffset = e.m_nOffset;
					d.InputSlotClass = slotClass;
					d.InstanceDataStepRate = nStepRate;
					vElements.push_back(d);
					break;
				}
				}
			}
		}
	}

	CVertexArray::CVertexArray()
	{
		PENUMBRA_PROFILE_FUNC();

		// Store the device context
		CGraphicsContext* pContext = CApplication::Get().GetWindow().GetGraphicsContext();
		m_spContext = pContext->GetContext();
	}

	void CVertexArray::Bind() const
	{
		PENUMBRA_PROFILE_FUNC();

		CRenderer::Submit([this]()
			{
				// Bind normal vertex buffers
				if (!m_vspVertexBuffers.empty()) {
					// Prepare arrays
					std::vector<ID3D11Buffer*> vpBuffers;
					std::vector<UINT> vnStrides;
					std::vector<UINT> vnOffsets;

					vpBuffers.reserve(m_vspVertexBuffers.size());
					vnStrides.reserve(m_vspVertexBuffers.size());
					vnOffsets.reserve(m_vspVertexBuffers.size());

					// Fill arrays
					for (const auto& vb : m_vspVertexBuffers) {
						// Buffer pointer
						vpBuffers.push_back(vb->GetBuffer().Get());
						// Stride from layout
						vnStrides.push_back(vb->GetLayout().GetStride());
						// Offset is always 0 for now
						vnOffsets.push_back(0);
					}

					{
						PENUMBRA_PROFILE_SCOPE("IASetVertexBuffers");
						m_spContext->IASetVertexBuffers(
							0,						// Start slot
							(UINT)vpBuffers.size(),	// Number of buffers
							vpBuffers.data(),		// Buffer pointers
							vnStrides.data(),		// Strides
							vnOffsets.data()		// Offsets
						);
					}
				}

				if (m_spInstanceBuffer) {
					// Slot is after normal vertex buffers
					const UINT nSlot = (UINT)m_vspVertexBuffers.size();
					// Stride from layout
					const UINT nStride = m_spInstanceBuffer->GetLayout().GetStride();
					// Offset is always 0 for now
					const UINT nOffset = 0;
					// Get buffer pointer
					ID3D11Buffer* const* ppBuffer = m_spInstanceBuffer->GetBuffer().GetAddressOf();

					{
						// Bind instance buffer
						PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::IASetVertexBuffers");
						m_spContext->IASetVertexBuffers(nSlot, 1, ppBuffer, &nStride, &nOffset);
					}
				}

				{
					// Set the input layout
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::IASetInputLayout");
					m_spContext->IASetInputLayout(m_spInputLayout.Get());
				}
			});

		// Bind index buffer
		if (m_spIndexBuffer) {
			m_spIndexBuffer->Bind();
		}
	}

	void CVertexArray::Unbind() const
	{
		PENUMBRA_PROFILE_FUNC();

		CRenderer::Submit([this]()
			{
				// Prepare unbind parameters
				const UINT nStride = 0;
				const UINT nOffset = 0;

				// Count of buffers to unbind
				UINT nCount = (UINT)m_vspVertexBuffers.size();
				if (m_spInstanceBuffer)
					nCount++;

				// Unbind all slots we used
				{
					// Unbind vertex buffers
					PENUMBRA_PROFILE_SCOPE("IASetVertexBuffers (unbind)");
					for (UINT i = 0; i < nCount; i++)
						m_spContext->IASetVertexBuffers(i, 1, nullptr, &nStride, &nOffset);
				}

				{
					// Unbind input layout
					PENUMBRA_PROFILE_SCOPE("IASetIndexBuffer (unbind)");
					m_spContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
				}
			});
	}

	void CVertexArray::CreateInputLayout(const Ref<CShader>& spVertexShader)
	{
		PENUMBRA_PROFILE_FUNC();

		// Prepare input element descriptions
		std::vector<D3D11_INPUT_ELEMENT_DESC> vElems;
		UINT nSlot = 0;

		// Get layout from all vertex buffers and create input element descriptions
		// Mat4s and Mat3s are split into multiple elements
		// Instance buffer elements are marked as per-instance data

		// Vertex buffers
		for (const auto& vb : m_vspVertexBuffers) {
			// Add elements for this vertex buffer
			Utils::AddInputElementsForLayout(vElems, vb->GetLayout(), nSlot, false);
			nSlot++;
		}

		// Instance buffer
		if (m_spInstanceBuffer) {
			Utils::AddInputElementsForLayout(vElems, m_spInstanceBuffer->GetLayout(), nSlot, true);
			nSlot++;
		}

		// Get device
		CGraphicsContext* pContext = CApplication::Get().GetWindow().GetGraphicsContext();
		ComPtr<ID3D11Device> spDevice = pContext->GetDevice();
		PENUMBRA_CORE_ASSERT(spDevice, "D3D11VertexBuffer: Could not get ID3D11Device from D3D11Context!");

		CRenderer::Submit([this, spDevice, vElems, spVertexShader]()
			{
				// Create the input layout
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateInputLayout");
				HRESULT hr = spDevice->CreateInputLayout(
					vElems.data(),			// Input element descriptions
					(UINT)vElems.size(),	// Number of elements
					spVertexShader->GetVertexBlob()->GetBufferPointer(),	// Pointer to shader bytecode
					spVertexShader->GetVertexBlob()->GetBufferSize(),		// Size of shader bytecode
					m_spInputLayout.GetAddressOf()	// Output input layout
				);
				
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Failed to create input layout.");
			});
	}

	void CVertexArray::AddVertexBuffer(const Ref<CVertexBuffer>& spVertexBuffer)
	{
		PENUMBRA_PROFILE_FUNC();

		PENUMBRA_CORE_ASSERT(spVertexBuffer->GetLayout().GetElements().size(), 
			"VertexArray: Vertex Buffer has no layout!");

		m_vspVertexBuffers.push_back(spVertexBuffer);
	}

	void CVertexArray::AddInstanceBuffer(const Ref<CVertexBuffer>& spVertexBuffer)
	{
		PENUMBRA_PROFILE_FUNC();

		PENUMBRA_CORE_ASSERT(spVertexBuffer->GetLayout().GetElements().size(), 
			"VertexArray: Instance Buffer has no layout!");

		m_spInstanceBuffer = spVertexBuffer;
	}

	void CVertexArray::SetIndexBuffer(const Ref<CIndexBuffer>& spIndexBuffer)
	{
		PENUMBRA_PROFILE_FUNC();

		m_spIndexBuffer = spIndexBuffer;
	}
}
