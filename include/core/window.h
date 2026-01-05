#pragma once

#include "core/base.h"
#include "events/event.h"

#include "renderer/renderer.h"
#include "renderer/graphics_context.h"

namespace penumbra
{
	struct WindowProps_t
	{
		std::string m_Title;
		uint32_t m_nWidth, m_nHeight;
		bool m_bFullscreen, m_bVSync;
	
		WindowProps_t(const std::string& title = "penumbra",
			uint32_t nWidth = 1280, uint32_t nHeight = 720,
			bool bFullscreen = false, bool bVSync = true)
			: m_Title(title), m_nWidth(nWidth), m_nHeight(nHeight), 
			m_bFullscreen(bFullscreen), m_bVSync(bVSync) { }
	};

	class CWindow
	{
	public:
		using EventCallbackFn = std::function<void(CEvent&)>;

		struct WindowData_t
		{
			std::string m_Title;
			uint32_t m_nWidth, m_nHeight;
			bool m_bFullscreen, m_bVSync;


			EventCallbackFn m_pfnEventCallback;
			bool m_bValidCallback = false;
		};
	public:
		CWindow(const WindowProps_t& props);
		~CWindow();

		void OnUpdate();

		uint32_t GetWidth() const { return m_WindowData.m_nWidth; }
		uint32_t GetHeight() const { return m_WindowData.m_nHeight; }
		std::pair<uint32_t, uint32_t> GetWidthHeight() const { return std::pair<uint32_t, uint32_t>(m_WindowData.m_nWidth, m_WindowData.m_nHeight); }

		void SetSize(uint32_t nWidth, uint32_t nHeight);
		void SetTitle(std::string title);

		void SetEventCallback(const EventCallbackFn& callback) { m_WindowData.m_pfnEventCallback = callback; m_WindowData.m_bValidCallback = true; }
		void SetVSync(bool bEnabled);
		bool IsVSync() const { return m_WindowData.m_bVSync; }
		void SetFullscreen(bool bEnabled);
		bool IsFullscreen() const { return m_WindowData.m_bFullscreen; }

		WindowData_t& GetWindowData() { return m_WindowData; }

		HWND GetHWND() const { return m_hWnd; }
		CGraphicsContext* GetGraphicsContext() const { return m_spGraphicsContext.get(); }
	private:
		void Init(const WindowProps_t& props);
		void Shutdown();
	private:
		HWND m_hWnd;
		Scope<CGraphicsContext> m_spGraphicsContext;

		WindowData_t m_WindowData;
	};
}