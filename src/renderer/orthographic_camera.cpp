#include "ppch.h"
#include "renderer/orthographic_camera.h"

#include "renderer/renderer.h"

namespace penumbra {

	COrthographicCamera::COrthographicCamera(float flLeft, float flRight, float flBottom, float flTop)
		: m_matProjectionMatrix(glm::orthoLH_ZO(flLeft, flRight, flBottom, flTop, kZNear, kZFar)), m_matViewMatrix(1.0f)
	{
		PENUMBRA_PROFILE_FUNC();

		m_matViewProjectionMatrix = m_matProjectionMatrix * m_matViewMatrix;
	}

	void COrthographicCamera::SetProjection(float flLeft, float flRight, float flBottom, float flTop)
	{
		PENUMBRA_PROFILE_FUNC();

		m_matProjectionMatrix = glm::orthoLH_ZO(flLeft, flRight, flBottom, flTop, kZNear, kZFar);
		m_matViewProjectionMatrix = m_matProjectionMatrix * m_matViewMatrix;
	}

	void COrthographicCamera::RecalculateViewMatrix()
	{
		PENUMBRA_PROFILE_FUNC();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(m_flRotation), glm::vec3(0, 0, 1));

		m_matViewMatrix = glm::inverse(transform);
		m_matViewProjectionMatrix = m_matProjectionMatrix * m_matViewMatrix;
	}
}