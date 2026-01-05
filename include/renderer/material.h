#pragma once

#include "renderer/constant_buffer.h"
#include "renderer/shader.h"

namespace penumbra
{
    class CTexture;

    enum class BlendMode {
        Opaque = 0,
        AlphaBlend,
        Additive,
        Multiply,
    };

    enum class RenderFace {
        Front = 0,
        Back,
        Both,
	};

    class CMaterial
    {
    public:
        ~CMaterial() = default;

        void InitializeUniforms();

        template<typename T>
        void Set(const std::string& name, const T& value)
        {
            if (m_spShader) {
                const ShaderUBuffer_t& layout = m_spShader->GetMaterialBuffer();
                for (const auto& var : layout.m_vecVariables) {
                    if (var.m_Name == name) {
                        std::memcpy(m_vecnUniformData.data() + var.m_nOffset, &value, sizeof(T));
                        return;
                    }
                }

                PENUMBRA_CORE_WARN("Material::Set - Uniform '{}' not found in shader", name);
            }
        }

        template<typename T>
        T Get(const std::string& name) const
        {
            if (m_spShader) {
                const ShaderUBuffer_t& layout = m_spShader->GetMaterialBuffer();
                for (const auto& var : layout.m_vecVariables) {
                    // Data is large enough
                    if (var.m_nOffset + sizeof(T) > m_vecnUniformData.size()) {
                        PENUMBRA_CORE_ERROR("Material::Get - Out-of-bounds access for uniform '{}'", name);
                        return T();
                    }

                    if (var.m_Name == name) {
                        T result;
                        std::memcpy(&result, m_vecnUniformData.data() + var.m_nOffset, sizeof(T));
                        return result;
                    }
                }

                PENUMBRA_CORE_WARN("Material::Get - Uniform '{}' not found in shader", name);
            }
            return T();
        }

        void SetTexture(const std::string& name, Ref<CTexture> spTexture);
        Ref<CTexture> GetTexture(const std::string& name) const;

        void UploadDataToGPU();
    public:
        static Ref<CMaterial> Create(Ref<CShader> spShader);
    public:
        Ref<CShader> m_spShader;

        BlendMode m_Blend = BlendMode::Opaque;
        RenderFace m_RenderFace = RenderFace::Front;
        bool m_bDepthWrite = true;
        bool m_bDepthTest = true;
        // TO-DO: stencil, etc. later
    private:
		Ref<CConstantBuffer> m_spUniformBuffer;

        std::vector<uint8_t> m_vecnUniformData;
        std::unordered_map<std::string, Ref<CTexture>> m_Textures;
    };
}