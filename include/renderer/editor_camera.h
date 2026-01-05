#pragma once

#include "renderer/camera.h"
#include "core/time.h"
#include "events/mouse_event.h"
#include "events/application_event.h"

namespace penumbra 
{
	class CEditorCamera : public CCamera
	{
	public:
		CEditorCamera() = default;
		CEditorCamera(float flFov, float flAspectRatio, float flNearClip, float flFarClip);

		void OnUpdate(CTime deltaTime);
		void OnEvent(CEvent& e);

		inline void SetViewportSize(float flWidth, float flHeight) { m_flViewportWidth = flWidth; m_flViewportHeight = flHeight; UpdateProjection(); }

		const glm::mat4& GetViewMatrix() const { return m_matViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_matProjection * m_matViewMatrix; }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; UpdateView(); }
		glm::quat GetOrientation() const;

		const float GetFOV() const { return m_flFOV; }
		const float GetAspectRatio() const { return m_flAspectRatio; }
		const float GetNearClip() const { return m_flNearClip; }
		const float GetFarClip() const { return m_flFarClip; }

		float GetPitch() const { return m_flPitch; }
		float GetYaw() const { return m_flYaw; }
	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(CMouseScrolledEvent& e);
		bool OnWindowResize(CWindowResizeEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float flDelta);

		glm::vec2 PanSpeed() const;
	private:
		// To-do: Lots of floating numbers to be converted to constants
		float m_flFOV = 45.0f, m_flAspectRatio = 1.778f, m_flNearClip = 0.1f, m_flFarClip = 1000.0f, m_flMoveSpeed = 10.0f;

		glm::mat4 m_matViewMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };

		float m_flPitch = 0.0f, m_flYaw = 0.0f;

		float m_flViewportWidth = 1920, m_flViewportHeight = 1080;
	};
}