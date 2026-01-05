#pragma once

#include "core/base.h"
#include "renderer/texture.h"
#include "math/math.h"

namespace penumbra
{
	class CSubTexture2D
	{
	public:
		CSubTexture2D(const Ref<CTexture2D>& spTexture, const glm::vec2& min, const glm::vec2& max);
		~CSubTexture2D() = default;

		const Ref<CTexture2D> GetTexture() const { return m_spTexture; }
		const glm::vec2* GetTexCoords() const { return m_TexCoords; }

		static Ref<CSubTexture2D> CreateFromCoords(const Ref<CTexture2D>& spTexture, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize = {1, 1});
	private:
		Ref<CTexture2D> m_spTexture;

		glm::vec2 m_TexCoords[4];
	};

}