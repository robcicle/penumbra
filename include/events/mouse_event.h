#pragma once

#include "events/event.h"
#include "core/mouse_codes.h"

namespace penumbra {

	class CMouseMovedEvent : public CEvent
	{
	public:
		CMouseMovedEvent(const float flX, const float flY)
			: m_flMouseX(flX), m_flMouseY(flY) {}

		float GetX() const { return m_flMouseX; }
		float GetY() const { return m_flMouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_flMouseX << ", " << m_flMouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
			EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT)
	private:
		float m_flMouseX, m_flMouseY;
	};

	class CMouseScrolledEvent : public CEvent
	{
	public:
		CMouseScrolledEvent(const float flXOffset, const float flYOffset)
			: m_flXOffset(flXOffset), m_flYOffset(flYOffset) {}

		float GetXOffset() const { return m_flXOffset; }
		float GetYOffset() const { return m_flYOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
			EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT)
	private:
		float m_flXOffset, m_flYOffset;
	};

	class CMouseButtonEvent : public CEvent
	{
	public:
		MouseCode GetMouseButton() const { return m_nButton; }

		EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT | EVENT_CATEGORY_MOUSE_BUTTON)
	protected:
		CMouseButtonEvent(const MouseCode nButton)
			: m_nButton(nButton) {}

		MouseCode m_nButton;
	};

	class CMouseButtonPressedEvent : public CMouseButtonEvent
	{
	public:
		CMouseButtonPressedEvent(const MouseCode nButton)
			: CMouseButtonEvent(nButton) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_nButton;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class CMouseButtonReleasedEvent : public CMouseButtonEvent
	{
	public:
		CMouseButtonReleasedEvent(const MouseCode nButton)
			: CMouseButtonEvent(nButton) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_nButton;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

}