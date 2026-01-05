#pragma once

#include "renderer/orthographic_camera.h"
#include "core/time.h"

#include "events/application_event.h"
#include "events/mouse_event.h"

namespace penumbra 
{
	struct OrthographicCameraBounds_t
	{
		float m_flLeft, m_flRight;
		float m_flBottom, m_flTop;

		float GetWidth() { return m_flRight - m_flLeft; }
		float GetHeight() { return m_flTop - m_flBottom; }
	};

	// TO-DO: This somehow missed the C prefix?
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float flAspectRatio, bool bRotation = false);
		~OrthographicCameraController() = default;

		void OnUpdate(CTime time);
		void OnEvent(CEvent& e, bool bIgnoreWindowResize = false);
		
		void OnResize(float flWidth, float flHeight);

		void SetPosition(const glm::vec3& position) { m_CameraPosition = position; m_Camera.SetPosition(m_CameraPosition); }
		glm::vec3& GetPosition() { return m_CameraPosition; }

		COrthographicCamera& GetCamera() { return m_Camera; }
		const COrthographicCamera& GetCamera() const { return m_Camera; }

		float GetZoomLevel() const { return m_flZoomLevel; }
		void SetZoomLevel(float flLevel) { m_flZoomLevel = flLevel; CalculateView(); }

		const OrthographicCameraBounds_t& GetBounds() const { return m_Bounds; }
	private:
		void CalculateView();

		bool OnMouseScrolled(CMouseScrolledEvent e);
		bool OnWindowResized(CWindowResizeEvent e);
	private:
		float m_flAspectRatio;
		float m_flZoomLevel = 1.0f;
		float m_flBaseCamSpeed = 5.0f;
		OrthographicCameraBounds_t m_Bounds;
		COrthographicCamera m_Camera;

		bool m_bRotation;
		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_flCameraRotation = 0;
		float m_flCameraTranslationSpeed = 0.1f, m_flCameraRotationSpeed = 30.0f;
	};
}