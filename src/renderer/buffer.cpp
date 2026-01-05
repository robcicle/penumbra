#include "ppch.h"
#include "renderer/buffer.h"

#include "core/application.h"
#include "renderer/renderer.h"

namespace penumbra
{
	static DXGI_FORMAT ShaderDataTypeToDXGIBaseType(ShaderDataType type)
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
		case ShaderDataType::Bool:      return DXGI_FORMAT_R8_UINT;
		}

		PENUMBRA_CORE_ASSERT(false, "Unknown ShaderDataType");
		return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	}

	CVertexBuffer::CVertexBuffer(uint32_t nSize)
	{
		PENUMBRA_PROFILE_FUNC();

		// Create the vertex buffer
		D3D11_BUFFER_DESC vertexBufferDescriptor{};
		vertexBufferDescriptor.ByteWidth = nSize;						// Size of the buffer in bytes
		vertexBufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;				// Dynamic so we can update it later
		vertexBufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// Vertex buffer bind flag
		vertexBufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // Required for mapping

		// Get the graphics context from the application window
		CGraphicsContext* pContext = CApplication::Get().GetWindow().GetGraphicsContext();
		PENUMBRA_CORE_ASSERT(pContext, "D3D11VertexBuffer: Could not get D3D11Context from Application::Window!");

		// Get the device to create the buffer
		ComPtr<ID3D11Device> spDevice = pContext->GetDevice();
		PENUMBRA_CORE_ASSERT(spDevice, "D3D11VertexBuffer: Could not get ID3D11Device from D3D11Context!");

		// Create the buffer on the render thread
		CRenderer::Submit([this, spDevice, vertexBufferDescriptor]()
			{
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateBuffer");
				spDevice->CreateBuffer(&vertexBufferDescriptor, nullptr, m_spVertexBuffer.GetAddressOf());
				PENUMBRA_CORE_ASSERT(m_spVertexBuffer, "D3D11VertexBuffer: Could not create VertexBuffer!");
			});

		// Get the device context for later use
		m_spContext = pContext->GetContext();
		PENUMBRA_CORE_ASSERT(m_spContext, "D3D11VertexBuffer: Could not get ID3D11DeviceContext from D3D11Context!");
	}

	CVertexBuffer::CVertexBuffer(void* pVertices, uint32_t nSize)
	{
		PENUMBRA_PROFILE_FUNC();
		
		// Create the vertex buffer
		D3D11_BUFFER_DESC vertexBufferDescriptor{};
		vertexBufferDescriptor.ByteWidth = nSize;										// Size of the buffer in bytes
		vertexBufferDescriptor.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;				// Immutable since we won't change the data
		vertexBufferDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;	// Vertex buffer bind flag

		// Get the graphics context from the application window
		CGraphicsContext* pContext = CApplication::Get().GetWindow().GetGraphicsContext();
		PENUMBRA_CORE_ASSERT(pContext, "D3D11VertexBuffer: Could not get D3D11Context from Application::Window!");

		// Get the device to create the buffer
		ComPtr<ID3D11Device> spDevice = pContext->GetDevice();
		PENUMBRA_CORE_ASSERT(spDevice, "D3D11VertexBuffer: Could not get ID3D11Device from D3D11Context!");

		// Create a copy of the vertex data to use on the render thread
		std::vector<uint8_t> vecDataCopy(reinterpret_cast<uint8_t*>(pVertices), reinterpret_cast<uint8_t*>(pVertices) + nSize);
		CRenderer::Submit([this, dataCopy = std::move(vecDataCopy), spDevice, vertexBufferDescriptor]()
			{
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateBuffer");

				// Create the subresource data
				D3D11_SUBRESOURCE_DATA vertexBufferData{};
				vertexBufferData.pSysMem = dataCopy.data();

				// Create the buffer
				spDevice->CreateBuffer(&vertexBufferDescriptor, &vertexBufferData, m_spVertexBuffer.GetAddressOf());
				PENUMBRA_CORE_ASSERT(m_spVertexBuffer, "D3D11VertexBuffer: Could not create VertexBuffer!");
			});

		// Get the device context for later use
		m_spContext = pContext->GetContext();
		PENUMBRA_CORE_ASSERT(m_spContext, "D3D11VertexBuffer: Could not get ID3D11DeviceContext from D3D11Context!");
	}

	void CVertexBuffer::Bind() const
	{
		PENUMBRA_PROFILE_FUNC();

		// Set the vertex buffer stride and offset
		const UINT nStrides = m_Layout.GetStride();
		const UINT nOffset = 0;

		CRenderer::Submit([this, nStrides, nOffset]()
			{
				// Bind the vertex buffer
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::IASetVertexBuffers");
				m_spContext->IASetVertexBuffers(0, 1, m_spVertexBuffer.GetAddressOf(), &nStrides, &nOffset);
			});
	}

	void CVertexBuffer::SetData(const void* pData, uint32_t nSize) const
	{
		PENUMBRA_PROFILE_FUNC();

		// Create a copy of the data to use on the render thread
		std::vector<uint8_t> vDataCopy(static_cast<const uint8_t*>(pData), static_cast<const uint8_t*>(pData) + nSize);
		CRenderer::Submit([this, dataCopy = std::move(vDataCopy)]()
			{
				// Map the vertex buffer
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				HRESULT hr = S_OK;
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::Map");
					hr = m_spContext->Map(m_spVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
					PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "D3D11VertexBuffer: Failed to map vertex buffer!");
				}

				// Copy the data to the mapped resource
				std::memcpy(mappedResource.pData, dataCopy.data(), dataCopy.size());

				// Unmap the vertex buffer
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::Unmap");
				m_spContext->Unmap(m_spVertexBuffer.Get(), 0);
			});
	}

	CIndexBuffer::CIndexBuffer(void* pIndices, uint32_t nSize)
		: m_nSize(nSize)
	{
		PENUMBRA_PROFILE_FUNC();

		// Create the index buffer description
		D3D11_BUFFER_DESC indexBufferDescriptor{};
		indexBufferDescriptor.ByteWidth = m_nSize;									// Size of the buffer in bytes
		indexBufferDescriptor.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;			// Immutable since we won't change the data
		indexBufferDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;	// Index buffer bind flag

		// Get the graphics context from the application window
		CGraphicsContext* pContext = CApplication::Get().GetWindow().GetGraphicsContext();
		PENUMBRA_CORE_ASSERT(pContext, "D3D11IndexBuffer: Could not get D3D11Context from Application::Window!");

		// Get the device to create the buffer
		ComPtr<ID3D11Device> spDevice = pContext->GetDevice();
		PENUMBRA_CORE_ASSERT(spDevice, "D3D11IndexBuffer: Could not get ID3D11Device from D3D11Context!");

		// Get the device context for later use
		m_spContext = pContext->GetContext();
		PENUMBRA_CORE_ASSERT(m_spContext, "D3D11IndexBuffer: Could not get ID3D11DeviceContext from D3D11Context!");

		// Create a copy of the index data to use on the render thread
		std::vector<uint8_t> vDataCopy(reinterpret_cast<uint8_t*>(pIndices), reinterpret_cast<uint8_t*>(pIndices) + nSize);
		CRenderer::Submit([this, spDevice, indexBufferDescriptor, dataCopy = std::move(vDataCopy)]() mutable
			{
				// Create the subresource data
				D3D11_SUBRESOURCE_DATA indexBufferData{};
				indexBufferData.pSysMem = dataCopy.data();

				// Create the buffer
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateBuffer");
				spDevice->CreateBuffer(&indexBufferDescriptor, &indexBufferData, m_spIndexBuffer.GetAddressOf());
				PENUMBRA_CORE_ASSERT(m_spIndexBuffer, "D3D11IndexBuffer: Could not create IndexBuffer!");
			});
	}

	void CIndexBuffer::Bind() const
	{
		PENUMBRA_PROFILE_FUNC();

		CRenderer::Submit([this]()
			{
				// Bind the index buffer
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::IASetIndexBuffer");
				m_spContext->IASetIndexBuffer(m_spIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			});
	}
}