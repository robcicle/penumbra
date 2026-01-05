#include "ppch.h"
#include "scene/scene_camera.h"

#include "renderer/renderer.h"

namespace penumbra
{
	CSceneCamera::CSceneCamera()
	{
		RecalculateProjection();
	}

	void CSceneCamera::SetPerspective(float flVerticalFov, float flNearClip, float flFarClip)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_flPerspectiveFOV = flVerticalFov;
		m_flPerspectiveNear = flNearClip;
		m_flPerspectiveFar = flFarClip;

		RecalculateProjection();
	}

	void CSceneCamera::SetOrthographic(float flSize, float flNearClip, float flFarClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_flOrthographicSize = flSize;
		m_flOrthographicNear = flNearClip;
		m_flOrthographicFar = flFarClip;

		RecalculateProjection();
	}

	void CSceneCamera::SetViewportSize(uint32_t nWidth, uint32_t nHeight)
	{
		PENUMBRA_CORE_ASSERT(nWidth > 0 && nHeight > 0);

		m_flAspectRatio = (float)nWidth / (float)nHeight;
		
		RecalculateProjection();
	}
	void CSceneCamera::RecalculateProjection()
	{

		switch (m_ProjectionType)
		{
			case ProjectionType::Perspective:
			{
				m_matProjection = glm::perspectiveLH_ZO(m_flPerspectiveFOV, m_flAspectRatio, 
					m_flPerspectiveNear, m_flPerspectiveFar);

				break;
			}
			case ProjectionType::Orthographic:
			{
				float orthoLeft = -m_flOrthographicSize * m_flAspectRatio * 0.5f;
				float orthoRight = m_flOrthographicSize * m_flAspectRatio * 0.5f;

				float orthoBottom = -m_flOrthographicSize * 0.5f;
				float orthoTop = m_flOrthographicSize * 0.5f;

				m_matProjection = glm::orthoLH_ZO(orthoLeft, orthoRight,
					orthoBottom, orthoTop, m_flOrthographicNear, m_flOrthographicFar);
			
				break;
			}
		}
	}
}