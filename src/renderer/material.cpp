#include "ppch.h"
#include "renderer/material.h"

#include "renderer/texture.h"
#include "renderer/renderer.h"

namespace penumbra
{
	void CMaterial::InitializeUniforms()
	{
		PENUMBRA_PROFILE_FUNC();

		if (m_spShader) {
			const ShaderUBuffer_t& layout = m_spShader->GetMaterialBuffer();
			m_vecnUniformData.resize(layout.m_nSize);
			m_spUniformBuffer = CreateRef<CConstantBuffer>(layout.m_nSize, layout.m_nBinding, ShaderType::SHADER_TYPE_PIXEL);
		}
	}

	void CMaterial::SetTexture(const std::string& name, Ref<CTexture> spTexture)
	{
		PENUMBRA_PROFILE_FUNC();

		if (m_spShader) {
			const auto& textures = m_spShader->GetMaterialBuffer().m_vecTextures;
			for (const auto& tex : textures) {
				if (tex.m_Name == name)
				{
					m_Textures[name] = spTexture;
					return;
				}
			}
		}
		PENUMBRA_CORE_WARN("Material::SetTexture - Texture '{}' not found in shader", name);
	}

	Ref<CTexture> CMaterial::GetTexture(const std::string& name) const
	{
		PENUMBRA_PROFILE_FUNC();

		auto it = m_Textures.find(name);
		if (it != m_Textures.end()) {
			return it->second;
		}

		return 0;
	}

	void CMaterial::UploadDataToGPU()
	{
		PENUMBRA_PROFILE_FUNC();

		if (m_spUniformBuffer) {
			m_spUniformBuffer->SetData(m_vecnUniformData.data(), static_cast<uint32_t>(m_vecnUniformData.size()));
			m_spUniformBuffer->Bind();
		}

		if (m_spShader) {
			const auto& textureSlots = m_spShader->GetMaterialBuffer().m_vecTextures;
			for (const auto& texInfo : textureSlots) {
				auto it = m_Textures.find(texInfo.m_Name);
				if (it != m_Textures.end()) {
					Ref<CTexture> texture = it->second;
					if (texture) {
						texture->Bind(texInfo.m_nBinding);
					}
					else {
						TextureSpecification_t spec{};
						CRenderer::GetColorTexture(glm::vec3(1.0f), spec)->Bind(texInfo.m_nBinding);
					}
				}
				else {
					TextureSpecification_t spec{};
					CRenderer::GetColorTexture(glm::vec3(1.0f), spec)->Bind(texInfo.m_nBinding);
				}
			}
		}
	}

	Ref<CMaterial> CMaterial::Create(Ref<CShader> spShader)
	{
		PENUMBRA_PROFILE_FUNC();

		Ref<CMaterial> material = CreateRef<CMaterial>();
		material->m_spShader = spShader;

		return material;
	}
}