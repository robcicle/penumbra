#pragma once

#include "core/base.h"

#include "renderer/buffer.h"

namespace penumbra
{
	class CVertexArray
	{
	public:
		CVertexArray();
		~CVertexArray() = default;

		void Bind() const;
		void Unbind() const;

		void CreateInputLayout(const Ref<CShader>& spVertexShader);

		void AddVertexBuffer(const Ref<CVertexBuffer>& spVertexBuffer);
		void AddInstanceBuffer(const Ref<CVertexBuffer>& spVertexBuffer);
		void SetIndexBuffer(const Ref<CIndexBuffer>& spIndexBuffer);

		const std::vector<Ref<CVertexBuffer>>& GetVertexBuffers() const { return m_vspVertexBuffers; };
		const Ref<CIndexBuffer>& GetIndexBuffer() const { return m_spIndexBuffer; };
	private:
		ComPtr<ID3D11InputLayout> m_spInputLayout;

		std::vector<Ref<CVertexBuffer>> m_vspVertexBuffers;
		Ref<CVertexBuffer> m_spInstanceBuffer;
		Ref<CIndexBuffer> m_spIndexBuffer;

		ComPtr<ID3D11DeviceContext> m_spContext;
	};
}