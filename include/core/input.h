#pragma once

#include "core/key_codes.h"
#include "core/mouse_codes.h"

namespace penumbra
{
	class CInput
	{
	public:
		static bool GetKeyPressed(const KeyCode nKey);
		static bool GetMouseButtonPressed(const MouseCode nButton);

		static void OnRawMouseMove(float dx, float dy);

		static glm::vec2 GetMousePosition();
		static glm::vec2 GetMouseRelativePosition();

		static float GetMouseX();
		static float GetMouseY();

		static bool SetMouseGrab(const bool bGrabbed);
		static void SetMouseCursor(const char* szCursorPath);
		static void ResetMouseCursor();

		static void WarpMouseInWindow(const float flX, const float flY);
	};
}