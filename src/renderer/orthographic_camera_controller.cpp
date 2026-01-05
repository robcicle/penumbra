#include "ppch.h"
#include "renderer/orthographic_camera_controller.h"

#include "core/input.h"
#include "core/key_codes.h"
#include "core/mouse_codes.h"

namespace penumbra
{
	OrthographicCameraController::OrthographicCameraController(float flAspectRatio, bool bRotation)
		: m_flAspectRatio(flAspectRatio), m_Bounds({ -m_flAspectRatio * m_flZoomLevel, m_flAspectRatio * m_flZoomLevel, -m_flZoomLevel, m_flZoomLevel }), m_Camera(m_Bounds.m_flLeft, m_Bounds.m_flRight, m_Bounds.m_flBottom, m_Bounds.m_flTop), m_bRotation(bRotation)
	{
		PENUMBRA_PROFILE_FUNC();

		m_Camera.SetPosition(m_CameraPosition);
		if (m_bRotation) {
			m_Camera.SetRotation(m_flCameraRotation);
		}
	}

	void OrthographicCameraController::OnUpdate(CTime time)
	{
		PENUMBRA_PROFILE_FUNC();

		if (CInput::GetMouseButtonPressed(Mouse::BUTTON_RIGHT)) {
			glm::vec2 mousePos = CInput::GetMouseRelativePosition();
			m_CameraPosition.x += (-mousePos.x * cos(glm::radians(m_flCameraRotation)) - mousePos.y * sin(glm::radians(m_flCameraRotation))) * m_flCameraTranslationSpeed;
			m_CameraPosition.y += (-mousePos.x * sin(glm::radians(m_flCameraRotation)) + mousePos.y * cos(glm::radians(m_flCameraRotation))) * m_flCameraTranslationSpeed;

			m_Camera.SetPosition(m_CameraPosition);
		}

		if (m_bRotation) {
			if (CInput::GetKeyPressed(Key::Q))
				m_flCameraRotation -= m_flCameraRotationSpeed * time;
			if (CInput::GetKeyPressed(Key::E))
				m_flCameraRotation += m_flCameraRotationSpeed * time;

			m_Camera.SetRotation(m_flCameraRotation);
		}

		m_flCameraTranslationSpeed = m_flZoomLevel * kTranslationSpeedFactor;
	}

	void OrthographicCameraController::OnEvent(CEvent& e, bool bIgnoreWindowResize)
	{
		PENUMBRA_PROFILE_FUNC();

		CEventDispatcher dispatcher(e);
		dispatcher.Dispatch<CMouseScrolledEvent>(PENUMBRA_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		if (!bIgnoreWindowResize) {
			dispatcher.Dispatch<CWindowResizeEvent>(PENUMBRA_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
		}
	}

	void OrthographicCameraController::OnResize(float width, float height)
	{
		PENUMBRA_PROFILE_FUNC();

		m_flAspectRatio = width / height;
		CalculateView();
	}

	void OrthographicCameraController::CalculateView()
	{
		PENUMBRA_PROFILE_FUNC();

		m_Bounds = { -m_flAspectRatio * m_flZoomLevel, m_flAspectRatio * m_flZoomLevel, -m_flZoomLevel, m_flZoomLevel };
		m_Camera.SetProjection(m_Bounds.m_flLeft, m_Bounds.m_flRight, m_Bounds.m_flBottom, m_Bounds.m_flTop);
	}

	bool OrthographicCameraController::OnMouseScrolled(CMouseScrolledEvent e)
	{
		PENUMBRA_PROFILE_FUNC();

		if (m_flZoomLevel - e.GetYOffset() <= kMinZoomLevel) {
			return false;
		}

		m_flZoomLevel -= e.GetYOffset();
		CalculateView();
		
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(CWindowResizeEvent e)
	{
		PENUMBRA_PROFILE_FUNC();

		OnResize((float)e.GetWidth(), (float)e.GetHeight());
		return false;
	}
}
