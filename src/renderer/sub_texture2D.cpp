#include "ppch.h"
#include "renderer/sub_texture2D.h"

namespace penumbra
{
	CSubTexture2D::CSubTexture2D(const Ref<CTexture2D>& spTexture, const glm::vec2& min, const glm::vec2& max)
		: m_spTexture(spTexture)
	{
		PENUMBRA_PROFILE_FUNC();

		m_TexCoords[0] = { min.x, min.y };
		m_TexCoords[1] = { max.x, min.y };
		m_TexCoords[2] = { max.x, max.y };
		m_TexCoords[3] = { min.x, max.y };
	}

	Ref<CSubTexture2D> CSubTexture2D::CreateFromCoords(const Ref<CTexture2D>& spTexture, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize)
	{
		PENUMBRA_PROFILE_FUNC();

		glm::vec2 min = { (coords.x * cellSize.x) / spTexture->GetWidth(), (coords.y * cellSize.y) / spTexture->GetHeight() };
		glm::vec2 max = { ((coords.x + spriteSize.x) * cellSize.x) / spTexture->GetWidth(), ((coords.y + spriteSize.y) * cellSize.y) / spTexture->GetHeight() };
		
		return CreateRef<CSubTexture2D>(spTexture, min, max);
	}
}
