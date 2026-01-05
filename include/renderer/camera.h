#pragma once

#include "math/math.h"

namespace penumbra 
{
	class CCamera
	{
	public:
		CCamera() = default;
		CCamera(const glm::mat4& matProjection)
			: m_matProjection(matProjection) {}

		virtual ~CCamera() = default;

		const glm::mat4& GetProjection() const { return m_matProjection; }
	protected:
		glm::mat4 m_matProjection = glm::mat4(1.0f);
	};
}