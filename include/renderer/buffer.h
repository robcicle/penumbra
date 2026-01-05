#pragma once

#include "renderer/shader.h"

namespace penumbra
{
	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:		return 4;
			case ShaderDataType::Float2:	return 4 * 2;
			case ShaderDataType::Float3:	return 4 * 3;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Mat3:		return 4 * 3 * 3;
			case ShaderDataType::Mat4:		return 4 * 4 * 4;
			case ShaderDataType::Int:		return 4;
			case ShaderDataType::Int2:		return 4 * 2;
			case ShaderDataType::Int3:		return 4 * 3;
			case ShaderDataType::Int4:		return 4 * 4;
			case ShaderDataType::Bool:		return 1;
		}
// This function will need moving at some point but because it's
// here the Sandbox App will compile it and get made that it doesn't
// know what an PENUMBRA_CORE is. So just putting this here.
#ifdef PENUMBRA_ENGINE
		PENUMBRA_CORE_ASSERT(false, "Unknown ShaderDataType!");
#endif
		return 0;
	}

	struct BufferElement_t
	{
		ShaderDataType m_Type;
		std::string m_Name;
		uint32_t m_nSize;
		size_t m_nOffset;
		bool m_bPerInstance = false;
		bool m_bNormalized;

		BufferElement_t() = default;

		BufferElement_t(ShaderDataType type, const std::string& name, bool bNormalized = false)
			: m_Type(type), m_Name(name), m_nSize(ShaderDataTypeSize(type)), m_nOffset(0), m_bNormalized(bNormalized)
		{}

		uint32_t GetComponentCount() const
		{
			switch (m_Type)
			{
				case ShaderDataType::Float:     return 1;
				case ShaderDataType::Float2:    return 2;
				case ShaderDataType::Float3:    return 3;
				case ShaderDataType::Float4:    return 4;
				case ShaderDataType::Mat3:      return 3; // 3* float3
				case ShaderDataType::Mat4:      return 4; // 4* float4
				case ShaderDataType::Int:       return 1;
				case ShaderDataType::Int2:      return 2;
				case ShaderDataType::Int3:      return 3;
				case ShaderDataType::Int4:      return 4;
				case ShaderDataType::Bool:      return 1;
			}

			//PENUMBRA_CORE_ASSERT(false, "Unknown ShaderDataType");
			return 0;
		}
	};

	class CBufferLayout
	{
	public:
		CBufferLayout() = default;
		~CBufferLayout() = default;

		CBufferLayout(const std::initializer_list<BufferElement_t>& elements ) 
			: m_vElements(elements) {
			CalculateOffsetsAndStride();
		}

		const uint32_t GetStride() const { return m_nStride; }
		const std::vector<BufferElement_t>& GetElements() const { return m_vElements; }

		std::vector<BufferElement_t>::iterator begin() { return m_vElements.begin(); }
		std::vector<BufferElement_t>::iterator end() { return m_vElements.end(); }

		std::vector<BufferElement_t>::const_iterator begin() const { return m_vElements.begin(); }
		std::vector<BufferElement_t>::const_iterator end() const { return m_vElements.end(); }
	private:
		void CalculateOffsetsAndStride() {
			size_t nOffset = 0;
			m_nStride = 0;
			for (auto& element : m_vElements) {
				element.m_nOffset = nOffset;
				nOffset += element.m_nSize;
				m_nStride += element.m_nSize;
			}
		}
	private:
		std::vector<BufferElement_t> m_vElements;
		uint32_t m_nStride = 0;
	};

	class CVertexBuffer
	{
	public:
		CVertexBuffer(uint32_t nSize);
		CVertexBuffer(void* pVertices, uint32_t nSize);
		~CVertexBuffer() = default;

		void Bind() const;
		void SetData(const void* pData, uint32_t nSize) const;

		void SetLayout(const CBufferLayout& layout) { m_Layout = layout; }
		const CBufferLayout& GetLayout() const { return m_Layout; }

		const ComPtr<ID3D11Buffer> GetBuffer() const { return m_spVertexBuffer; }
	private:
		ComPtr<ID3D11Buffer> m_spVertexBuffer;

		ComPtr<ID3D11DeviceContext> m_spContext;

		CBufferLayout m_Layout;
	};

	// Only 32 bit index buffers are supported!
	class CIndexBuffer
	{
	public:
		CIndexBuffer(void* pIndices, uint32_t nSize);
		~CIndexBuffer() = default;

		void Bind() const;

		const uint32_t GetCount() const { return m_nSize / sizeof(uint32_t); }
	private:
		ComPtr<ID3D11Buffer> m_spIndexBuffer;
		uint32_t m_nSize;

		ComPtr<ID3D11DeviceContext> m_spContext;
	};
}