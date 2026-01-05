#include "ppch.h"
#include "core/input.h"

#include "core/application.h"

namespace penumbra
{
	static glm::vec2 s_RawDelta = { 0, 0 };
	static bool  s_MouseGrabbed = false;

	bool CInput::GetKeyPressed(const KeyCode nKey)
	{
		PENUMBRA_PROFILE_FUNC();

		return (GetAsyncKeyState((int)nKey) & 0x8000) != 0;
	}

	bool CInput::GetMouseButtonPressed(const MouseCode nButton)
	{
		PENUMBRA_PROFILE_FUNC();

		return (GetAsyncKeyState(nButton) & 0x8000) != 0;
	}

	void CInput::OnRawMouseMove(float dx, float dy)
	{
		PENUMBRA_PROFILE_FUNC();

		s_RawDelta.x += dx;
		s_RawDelta.y += dy;
	}

	glm::vec2 CInput::GetMousePosition()
	{
		PENUMBRA_PROFILE_FUNC();

		POINT p;
		GetCursorPos(&p);

		// Convert to client coordinates
		HWND hwnd = (HWND)CApplication::Get().GetWindow().GetHWND();
		ScreenToClient(hwnd, &p);

		return { (float)p.x, (float)p.y };
	}
	glm::vec2 CInput::GetMouseRelativePosition()
	{
		PENUMBRA_PROFILE_FUNC();

		glm::vec2 delta = s_RawDelta;
		s_RawDelta = { 0, 0 };
		return delta;
	}
	float CInput::GetMouseX()
	{
		return GetMousePosition().x;
	}
	float CInput::GetMouseY()
	{
		return GetMousePosition().y;
	}
	bool CInput::SetMouseGrab(const bool bGrabbed)
	{
		PENUMBRA_PROFILE_FUNC();

		HWND hwnd = (HWND)CApplication::Get().GetWindow().GetHWND();

		if (bGrabbed)
		{
			RECT rect;
			GetClientRect(hwnd, &rect);
			MapWindowPoints(hwnd, nullptr, reinterpret_cast<LPPOINT>(&rect), 2);
			ClipCursor(&rect);
			ShowCursor(FALSE);
			s_MouseGrabbed = true;
		}
		else
		{
			ClipCursor(nullptr);
			ShowCursor(TRUE);
			s_MouseGrabbed = false;
		}

		return true;
	}
	void CInput::SetMouseCursor(const char* szCursorPath)
	{
		PENUMBRA_PROFILE_FUNC();

	}
	void CInput::ResetMouseCursor()
	{
		PENUMBRA_PROFILE_FUNC();

	}
	void CInput::WarpMouseInWindow(const float flX, const float flY)
	{
		PENUMBRA_PROFILE_FUNC();

		HWND hwnd = (HWND)CApplication::Get().GetWindow().GetHWND();

		POINT p = { (LONG)flX, (LONG)flY };
		ClientToScreen(hwnd, &p);
		SetCursorPos(p.x, p.y);
	}
}