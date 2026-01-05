#include "ppch.h"
#include "core/window.h"

#include "core/application.h"
#include "core/key_codes.h"
#include "core/input.h"

#include "events/application_event.h"
#include "events/key_event.h"
#include "events/mouse_event.h"

#include <windowsx.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace penumbra
{
	static bool s_bWin32Registered = false;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	CWindow::CWindow(const WindowProps_t& props)
	{
		PENUMBRA_PROFILE_FUNC();

		Init(props);
	}

	CWindow::~CWindow()
	{
		PENUMBRA_PROFILE_FUNC();

		Shutdown();
	}

	void CWindow::SetSize(uint32_t nWidth, uint32_t nHeight)
	{
		PENUMBRA_PROFILE_FUNC();

		RECT rect = { 0, 0, (LONG)nWidth, (LONG)nHeight };
		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
		SetWindowPos(m_hWnd, nullptr, CW_USEDEFAULT, CW_USEDEFAULT,
			rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE);

		m_WindowData.m_nWidth = nWidth;
		m_WindowData.m_nHeight = nHeight;

		CWindowResizeEvent e(nWidth, nHeight, m_WindowData.m_bFullscreen);
		m_WindowData.m_pfnEventCallback(e);
	}

	void CWindow::SetTitle(std::string title)
	{
		PENUMBRA_PROFILE_FUNC();

		SetWindowTextA(m_hWnd, title.c_str());
		m_WindowData.m_Title = title;
	}

	void CWindow::SetVSync(bool bEnabled)
	{
		PENUMBRA_PROFILE_FUNC();

		m_WindowData.m_bVSync = bEnabled;
		m_spGraphicsContext->SetVSync(m_WindowData.m_bVSync);
	}

	void CWindow::SetFullscreen(bool bEnabled)
	{
		PENUMBRA_PROFILE_FUNC();

		m_WindowData.m_bFullscreen = bEnabled;

		if (bEnabled)
		{
			// --- Change display mode to desired resolution ---
			DEVMODE dm = {};
			dm.dmSize = sizeof(dm);
			dm.dmPelsWidth = m_WindowData.m_nWidth;
			dm.dmPelsHeight = m_WindowData.m_nHeight;
			dm.dmBitsPerPel = 32;
			dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

			if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
				PENUMBRA_CORE_WARN("Failed to switch to fullscreen mode {0}x{1}",
					m_WindowData.m_nWidth, m_WindowData.m_nHeight);
				m_WindowData.m_bFullscreen = false;
				return;
			}

			// --- Borderless popup window ---
			SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
			SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);
			SetWindowPos(m_hWnd, HWND_TOP,
				0, 0,
				m_WindowData.m_nWidth, m_WindowData.m_nHeight,
				SWP_FRAMECHANGED | SWP_SHOWWINDOW);

			// Notify the engine
			CWindowResizeEvent e(m_WindowData.m_nWidth, m_WindowData.m_nHeight, true);
			if (m_WindowData.m_bValidCallback) {
				m_WindowData.m_pfnEventCallback(e);
			}
		}
		else
		{
			// --- Revert display mode ---
			ChangeDisplaySettings(nullptr, 0);

			// Restore windowed style
			SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
			SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, 0);

			// --- Position window safely at center of the primary monitor ---
			MONITORINFO mi = { sizeof(mi) };
			GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);
			int screenWidth = mi.rcWork.right - mi.rcWork.left;
			int screenHeight = mi.rcWork.bottom - mi.rcWork.top;

			int windowWidth = (int)m_WindowData.m_nWidth;
			int windowHeight = (int)m_WindowData.m_nHeight;
			int posX = mi.rcWork.left + (screenWidth - windowWidth) / 2;
			int posY = mi.rcWork.top + (screenHeight - windowHeight) / 2;

			RECT rect = { 0, 0, windowWidth, windowHeight };
			AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
			int adjustedWidth = rect.right - rect.left;
			int adjustedHeight = rect.bottom - rect.top;

			SetWindowPos(m_hWnd, HWND_NOTOPMOST,
				posX, posY,
				adjustedWidth, adjustedHeight,
				SWP_FRAMECHANGED | SWP_SHOWWINDOW);

			// Bring window to foreground
			ShowWindow(m_hWnd, SW_RESTORE);
			SetForegroundWindow(m_hWnd);

			// Send resize event
			if (m_WindowData.m_bValidCallback) {
				CWindowResizeEvent e(m_WindowData.m_nWidth, m_WindowData.m_nHeight, false);
				m_WindowData.m_pfnEventCallback(e);
			}
		}
	}

	void CWindow::Init(const WindowProps_t& props)
	{
		PENUMBRA_PROFILE_FUNC();

		m_WindowData.m_Title = props.m_Title;
		m_WindowData.m_nWidth = props.m_nWidth;
		m_WindowData.m_nHeight = props.m_nHeight;
		m_WindowData.m_bFullscreen = props.m_bFullscreen;
		m_WindowData.m_bVSync = props.m_bVSync;

		HINSTANCE hInstance = GetModuleHandle(nullptr);

		if (!s_bWin32Registered) {
			WNDCLASSEXA wc = {};
			wc.cbSize = sizeof(WNDCLASSEXA);
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wc.lpfnWndProc = WndProc;
			wc.hInstance = hInstance;
			wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wc.lpszClassName = kWindowClassName;
			RegisterClassExA(&wc);
			s_bWin32Registered = true;
		}

		DWORD style = WS_OVERLAPPEDWINDOW;
		DWORD exStyle = 0;

		if (m_WindowData.m_bFullscreen) {
			// --- Switch Display Mode ---
			DEVMODE dm = {};
			dm.dmSize = sizeof(dm);
			dm.dmPelsWidth = m_WindowData.m_nWidth;
			dm.dmPelsHeight = m_WindowData.m_nHeight;
			dm.dmBitsPerPel = 32;
			dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

			// This changes the system display mode.
			if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				PENUMBRA_CORE_WARN("Failed to set display mode {0}x{1}, falling back to windowed mode.",
					m_WindowData.m_nWidth, m_WindowData.m_nHeight);
				m_WindowData.m_bFullscreen = false;
			}
			else
			{
				style = WS_POPUP;
				exStyle = WS_EX_APPWINDOW;
			}
		}

		RECT rect = { 0, 0, (LONG)m_WindowData.m_nWidth, (LONG)m_WindowData.m_nHeight };
		AdjustWindowRect(&rect, style, FALSE);

		m_hWnd = CreateWindowExA(
			exStyle, kWindowClassName, m_WindowData.m_Title.c_str(), style,
			0, 0,
			rect.right - rect.left, rect.bottom - rect.top,
			nullptr, nullptr, hInstance, this);

		// Register the mouse and keyboard for raw input
		RAWINPUTDEVICE rid[2];

		// Mouse
		rid[0].usUsagePage = 0x01;	// Generic Desktop Controls
		rid[0].usUsage = 0x02;		// Mouse
		rid[0].dwFlags = 0;			// No flags
		rid[0].hwndTarget = m_hWnd;	// Handle to target window

		// Keyboard
		rid[1].usUsagePage = 0x01;	// Generic Desktop Controls
		rid[1].usUsage = 0x06;		// Keyboard
		rid[1].dwFlags = 0;			// No flags
		rid[1].hwndTarget = m_hWnd;	// Handle to target window

		// Register the devices
		if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) {
			PENUMBRA_CORE_ERROR("Window: Failed to register raw input devices!");
		}

		m_spGraphicsContext = CreateScope<CGraphicsContext>();
		
		ShowWindow(m_hWnd, SW_SHOW);
		UpdateWindow(m_hWnd);

		SetVSync(m_WindowData.m_bVSync);
	}

	void CWindow::Shutdown()
	{
		PENUMBRA_PROFILE_FUNC();

		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
	}

	void CWindow::OnUpdate()
	{
		PENUMBRA_PROFILE_FUNC();

		// Needs to be called each frame to update mouse position
		CInput::GetMouseRelativePosition();

		m_spGraphicsContext->SwapBuffers();

		MSG msg;
		while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				CWindowCloseEvent e;
				m_WindowData.m_pfnEventCallback(e);
				return;
			}

			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}


	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		PENUMBRA_PROFILE_FUNC();

		CWindow* pWindow = nullptr;

		if (msg == WM_NCCREATE)
		{
			CREATESTRUCTA* cs = reinterpret_cast<CREATESTRUCTA*>(lParam);
			pWindow = reinterpret_cast<CWindow*>(cs->lpCreateParams);
			SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)pWindow);
		}
		else
		{
			pWindow = reinterpret_cast<CWindow*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
		}

		if (!pWindow)
			return DefWindowProcA(hwnd, msg, wParam, lParam);

		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
			return true;

		auto& data = pWindow->GetWindowData();
		switch (msg)
		{
		case WM_CLOSE:
		{
			CWindowCloseEvent e;
			data.m_pfnEventCallback(e);
			break;
		}
		case WM_SIZE:
		{
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			data.m_nWidth = width;
			data.m_nHeight = height;
			CWindowResizeEvent e(width, height, data.m_bFullscreen);
			if (data.m_bValidCallback)
				data.m_pfnEventCallback(e);
			break;
		}
		case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP)wParam;
			UINT count = DragQueryFileA(hDrop, 0xFFFFFFFF, nullptr, 0);
			std::vector<std::filesystem::path> filepaths;
			filepaths.reserve(count);

			for (UINT i = 0; i < count; i++)
			{
				char szFile[MAX_PATH];
				DragQueryFileA(hDrop, i, szFile, MAX_PATH);
				filepaths.emplace_back(szFile);
			}
			DragFinish(hDrop);

			CWindowDropEvent e(std::move(filepaths));
			if (data.m_bValidCallback)
				data.m_pfnEventCallback(e);
			break;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			CKeyPressedEvent e((int)wParam, (lParam >> 30) & 1);
			if (data.m_bValidCallback)
				data.m_pfnEventCallback(e);
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			CKeyReleasedEvent e((int)wParam);
			if (data.m_bValidCallback)
				data.m_pfnEventCallback(e);
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			CMouseButtonPressedEvent e((int)wParam);
			if (data.m_bValidCallback)
				data.m_pfnEventCallback(e);
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			CMouseButtonReleasedEvent e((int)wParam);
			if (data.m_bValidCallback)
				data.m_pfnEventCallback(e);
			break;
		}
		case WM_MOUSEWHEEL:
		{
			float deltaY = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
			CMouseScrolledEvent e(0.0f, deltaY);
			if (data.m_bValidCallback)
				data.m_pfnEventCallback(e);
			break;
		}
		case WM_MOUSEMOVE:
		{
			float x = (float)GET_X_LPARAM(lParam);
			float y = (float)GET_Y_LPARAM(lParam);
			CMouseMovedEvent e(x, y);
			if (data.m_bValidCallback)
				data.m_pfnEventCallback(e);
			break;
		}
		case WM_INPUT:
		{
			UINT dwSize = 0;
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));

			std::vector<BYTE> lpb(dwSize);
			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb.data(), &dwSize, sizeof(RAWINPUTHEADER)) == dwSize) {
				RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb.data());

				if (raw->header.dwType == RIM_TYPEMOUSE) {
					const RAWMOUSE& rm = raw->data.mouse;
					// Raw delta movement
					LONG dx = rm.lLastX;
					LONG dy = rm.lLastY;

					// Currently we just forward this to the input system
					/*CMouseMovedEvent e(dx, dy);
					if (data.m_bValidCallback)
						data.m_pfnEventCallback(e);*/

					CInput::OnRawMouseMove((float)dx, (float)dy);
				}
				else if (raw->header.dwType == RIM_TYPEKEYBOARD) {
					// Currently do nothing with raw keyboard input
					// but can be implemented at a later time.
					/*const RAWKEYBOARD& rk = raw->data.keyboard;
					USHORT vkey = rk.VKey;
					bool pressed = !(rk.Flags & RI_KEY_BREAK);

					if (pressed) {
						CKeyPressedEvent e((int)vkey, false);
						if (data.m_bValidCallback)
							data.m_pfnEventCallback(e);
					}
					else {
						CKeyReleasedEvent e((int)vkey);
						if (data.m_bValidCallback)
							data.m_pfnEventCallback(e);
					}*/
				}
			}
			break;
		}
		}

		return DefWindowProcA(hwnd, msg, wParam, lParam);
	}
}