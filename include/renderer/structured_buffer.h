#pragma once

#include "renderer/shader_type.h"

namespace penumbra
{
	struct StructuredBufferSpecification_t
    {
        uint32_t m_nElementSize = 0;
        uint32_t m_nElementCount = 0;
        uint32_t m_nSRVBinding = 0;
		uint32_t m_nUAVBinding = 0;
		ShaderType m_ShaderType = ShaderType::SHADER_TYPE_NONE;
		bool m_bAllowUAV = false;
		bool m_bUseCounter = false;
        bool m_bAllowRawUAV = false;
    };

    class CStructuredBuffer
    {
    public:
        CStructuredBuffer(const StructuredBufferSpecification_t& spec);
        ~CStructuredBuffer() = default;

        void BindSRV(ShaderType stage = ShaderType::SHADER_TYPE_NONE);
        void UnbindSRV(ShaderType stage = ShaderType::SHADER_TYPE_NONE);
        void BindUAV();
        void UnbindUAV();
        void SetData(const void* pData, uint32_t count);

		bool HasCounter() const { return m_Spec.m_bUseCounter; }
		void ResetUAVCounter(uint32_t value);
    private:
        StructuredBufferSpecification_t m_Spec;

        ComPtr<ID3D11Buffer> m_spBuffer;
        ComPtr<ID3D11ShaderResourceView> m_spSRV;
        ComPtr<ID3D11UnorderedAccessView> m_spUAV;

        ComPtr<ID3D11DeviceContext> m_spContext;
    };
}