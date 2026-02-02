#include "ppch.h"
#include "renderer/editor_camera.h"

#include "core/input.h"
#include "core/key_codes.h"
#include "core/mouse_codes.h"
#include "renderer/renderer.h"

namespace penumbra 
{
	CEditorCamera::CEditorCamera(float flFov, float flAspectRatio, float flNearClip, float flFarClip)
		: m_flFOV(flFov), m_flAspectRatio(flAspectRatio), m_flNearClip(flNearClip), m_flFarClip(flFarClip), CCamera(glm::perspectiveLH_ZO(glm::radians(m_flFOV), m_flAspectRatio, m_flNearClip, m_flFarClip))
	{
		PENUMBRA_PROFILE_FUNC();

		UpdateView();
	}

	void CEditorCamera::UpdateProjection()
	{
		PENUMBRA_PROFILE_FUNC();

		m_flAspectRatio = m_flViewportWidth / m_flViewportHeight;

		m_matProjection = glm::perspectiveLH_ZO(glm::radians(m_flFOV), m_flAspectRatio, m_flNearClip, m_flFarClip);
	}

	void CEditorCamera::UpdateView()
	{
		PENUMBRA_PROFILE_FUNC();

		glm::quat quatOrientation = GetOrientation();
		m_matViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(quatOrientation);
		m_matViewMatrix = glm::inverse(m_matViewMatrix);
	}

	glm::vec2 CEditorCamera::PanSpeed() const
	{
		PENUMBRA_PROFILE_FUNC();

		float x = std::min(m_flViewportWidth / kPanViewportDivisor, kPanSpeedMax);
		float xFactor = kPanCoeffA * (x * x) - kPanCoeffB * x + kPanCoeffC;

		float y = std::min(m_flViewportHeight / kPanViewportDivisor, kPanSpeedMax);
		float yFactor = kPanCoeffA * (y * y) - kPanCoeffB * y + kPanCoeffC;

		return glm::vec2{ xFactor, yFactor };
	}

	void CEditorCamera::OnUpdate(CTime ts)
	{
		PENUMBRA_PROFILE_FUNC();

		if (CInput::GetMouseButtonPressed(Mouse::BUTTON_RIGHT)) {
			glm::vec2 delta = CInput::GetMouseRelativePosition() * kMouseSensitivity;

			float speed = m_flMoveSpeed * ts;
			float boostMultiplier = (CInput::GetKeyPressed(Key::LEFT_SHIFT)) ? kMoveSpeedBoostMultiplier : kMoveSpeedMin;

			//CInput::SetMouseCursor("resources\\images\\cursor1.bmp");

			MouseRotate(delta);

			if (CInput::GetKeyPressed(Key::W))
				m_Position += GetForwardDirection() * speed * boostMultiplier;
			if (CInput::GetKeyPressed(Key::S))
				m_Position -= GetForwardDirection() * speed * boostMultiplier;
			if (CInput::GetKeyPressed(Key::A))
				m_Position -= GetRightDirection() * speed * boostMultiplier;
			if (CInput::GetKeyPressed(Key::D))
				m_Position += GetRightDirection() * speed * boostMultiplier;
			if (CInput::GetKeyPressed(Key::Q)) // Move Down
				m_Position -= GetUpDirection() * speed * boostMultiplier;
			if (CInput::GetKeyPressed(Key::E)) // Move Up
				m_Position += GetUpDirection() * speed * boostMultiplier;
		}
		else if (CInput::GetMouseButtonPressed(Mouse::BUTTON_MIDDLE)) {
			glm::vec2 delta = CInput::GetMouseRelativePosition() * kMouseSensitivity;
			MousePan(delta);
		}
		else {
			CInput::ResetMouseCursor();
		}

		UpdateView();
	}

	void CEditorCamera::OnEvent(CEvent& e)
	{
		PENUMBRA_PROFILE_FUNC();

		CEventDispatcher dispatcher(e);
		dispatcher.Dispatch<CMouseScrolledEvent>(PENUMBRA_BIND_EVENT_FN(CEditorCamera::OnMouseScroll));
	}

	bool CEditorCamera::OnMouseScroll(CMouseScrolledEvent& e)
	{
		PENUMBRA_PROFILE_FUNC();

		if (CInput::GetMouseButtonPressed(Mouse::BUTTON_RIGHT)) {
			m_flMoveSpeed += e.GetYOffset();

			if (m_flMoveSpeed < kMoveSpeedMin) {
				m_flMoveSpeed = kMoveSpeedMin;
			}
		}
		else {
			float delta = e.GetYOffset();
			MouseZoom(delta);
			UpdateView();
		}
		return false;
	}

	void CEditorCamera::MousePan(const glm::vec2& delta)
	{
		PENUMBRA_PROFILE_FUNC();

		glm::vec2 panSpeed = PanSpeed();
		m_Position += -GetRightDirection() * delta.x * panSpeed.x * m_flMoveSpeed;
		m_Position += GetUpDirection() * delta.y * panSpeed.y * m_flMoveSpeed;
	}

	void CEditorCamera::MouseRotate(const glm::vec2& delta)
	{
		PENUMBRA_PROFILE_FUNC();

		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_flYaw -= yawSign * delta.x * kRotationSpeed;
		m_flPitch -= delta.y * kRotationSpeed;

		// Clamp pitch to avoid flipping
		constexpr float pitchLimit = glm::radians(89.0f);
		m_flPitch = glm::clamp(m_flPitch, -pitchLimit, pitchLimit);
	}

	void CEditorCamera::MouseZoom(float flDelta)
	{
		PENUMBRA_PROFILE_FUNC();

		m_Position += GetForwardDirection() * flDelta * 1.0f;
	}

	glm::vec3 CEditorCamera::GetUpDirection() const
	{
		PENUMBRA_PROFILE_FUNC();

		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 CEditorCamera::GetRightDirection() const
	{
		PENUMBRA_PROFILE_FUNC();

		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 CEditorCamera::GetForwardDirection() const
	{
		PENUMBRA_PROFILE_FUNC();

		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, 1.0f));
	}

	glm::quat CEditorCamera::GetOrientation() const
	{
		PENUMBRA_PROFILE_FUNC();

		return glm::quat(glm::vec3(-m_flPitch, -m_flYaw, 0.0f));
	}
}