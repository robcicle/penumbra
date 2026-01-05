#pragma once

#include "events/event.h"
#include "core/key_codes.h"

namespace penumbra {

	class CKeyEvent : public CEvent
	{
	public:
		KeyCode GetKeyCode() const { return m_nKeyCode; }

		EVENT_CLASS_CATEGORY(EVENT_CATEGORY_KEYBOARD | EVENT_CATEGORY_INPUT)
	protected:
		CKeyEvent(const KeyCode nKeycode)
			: m_nKeyCode(nKeycode) {}

		KeyCode m_nKeyCode;
	};

	class CKeyPressedEvent : public CKeyEvent
	{
	public:
		CKeyPressedEvent(const KeyCode nKeycode, bool bIsRepeat = false)
			: CKeyEvent(nKeycode), m_bIsRepeat(bIsRepeat) {}

		bool IsRepeat() const { return m_bIsRepeat; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_nKeyCode << " (repeat = " << m_bIsRepeat << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		bool m_bIsRepeat;
	};

	class CKeyReleasedEvent : public CKeyEvent
	{
	public:
		CKeyReleasedEvent(const KeyCode nKeycode)
			: CKeyEvent(nKeycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_nKeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};
}