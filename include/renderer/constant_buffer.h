#pragma once

#include "core/base.h"
#include "renderer/shader_type.h"

namespace penumbra 
{
	class CConstantBuffer
	{
	public:
		CConstantBuffer(uint32_t nSize, uint32_t nBinding, ShaderType shaderType);
		~CConstantBuffer() = default;

		void Bind(ShaderType stage = ShaderType::SHADER_TYPE_NONE);
		void SetData(const void* pData, uint32_t nSize, uint32_t nOffset = 0);
	private:
		uint32_t m_nBinding;
		ShaderType m_ShaderType;

		ComPtr<ID3D11Buffer> m_spConstantBuffer;

		ComPtr<ID3D11DeviceContext> m_spContext;
	};

}
