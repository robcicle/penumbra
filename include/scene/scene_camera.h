#pragma once

#include "renderer/camera.h"

namespace penumbra
{
	class CSceneCamera : public CCamera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };;
	public:
		CSceneCamera();
		virtual ~CSceneCamera() = default;

		void SetPerspective(float flVerticalFov, float flNearClip, float flFarClip);
		void SetOrthographic(float flSize, float flNearClip, float flFarClip);

		void SetViewportSize(uint32_t nWidth, uint32_t nHeight);

		float GetPerspectiveVerticalFOV() const { return m_flPerspectiveFOV; }
		void SetPerspectiveVerticalFOV(float flVerticalFov) { m_flPerspectiveFOV = flVerticalFov; RecalculateProjection(); }
		float GetPerspectiveNearClip() const { return m_flPerspectiveNear; }
		void SetPerspectiveNearClip(float flNearClip) { m_flPerspectiveNear = flNearClip; RecalculateProjection(); }
		float GetPerspectiveFarClip() const { return m_flPerspectiveFar; }
		void SetPerspectiveFarClip(float flFarClip) { m_flPerspectiveFar = flFarClip; RecalculateProjection(); }
		
		float GetOrthographicSize() const { return m_flOrthographicSize; }
		void SetOrthographicSize(float flSize) { m_flOrthographicSize = flSize; RecalculateProjection(); }
		float GetOrthographicNearClip() const { return m_flOrthographicNear; }
		void SetOrthographicNearClip(float flNearClip) { m_flOrthographicNear = flNearClip; RecalculateProjection(); }
		float GetOrthographicFarClip() const { return m_flOrthographicFar; }
		void SetOrthographicFarClip(float flFarClip) { m_flOrthographicFar = flFarClip; RecalculateProjection(); }

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }
	private:
		void RecalculateProjection();
	private:
		ProjectionType m_ProjectionType = ProjectionType::Perspective;

		float m_flPerspectiveFOV = glm::radians(60.0f);
		float m_flPerspectiveNear = 0.3f, m_flPerspectiveFar = 2048.0f;

		float m_flOrthographicSize = 5.0f;
		float m_flOrthographicNear = 0.0f, m_flOrthographicFar = 1000.0f;

		float m_flAspectRatio = 1.0f;
	};
}