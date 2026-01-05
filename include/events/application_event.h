#pragma once

#include "events/event.h"

namespace penumbra {

	class CWindowResizeEvent : public CEvent
	{
	public:
		CWindowResizeEvent(uint32_t nWidth, uint32_t nHeight, bool bFullscreen)
			: m_nWidth(nWidth), m_nHeight(nHeight), m_bFullscreen(bFullscreen) {}

		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }
		std::pair<uint32_t, uint32_t> GetWidthHeight() const { return std::make_pair(m_nWidth, m_nHeight); }
		uint32_t IsFullscreen() const { return m_bFullscreen; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_nWidth << ", " << m_nHeight << " | Fullscreen: " << m_bFullscreen ? "true" : "false";
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
	private:
		uint32_t m_nWidth, m_nHeight;
		bool m_bFullscreen;
	};

	class CWindowCloseEvent : public CEvent
	{
	public:
		CWindowCloseEvent() = default;

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
	};

	class CAppTickEvent : public CEvent
	{
	public:
		CAppTickEvent() = default;

		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
	};

	class CAppUpdateEvent : public CEvent
	{
	public:
		CAppUpdateEvent() = default;

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
	};

	class CAppRenderEvent : public CEvent
	{
	public:
		CAppRenderEvent() = default;

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
	};

	class CWindowDropEvent : public CEvent
	{
	public:
		CWindowDropEvent(const std::vector<std::filesystem::path>& paths)
			: m_Paths(paths) {
		}

		CWindowDropEvent(std::vector<std::filesystem::path>&& paths)
			: m_Paths(std::move(paths)) {
		}

		const std::vector<std::filesystem::path>& GetPaths() const { return m_Paths; }

		EVENT_CLASS_TYPE(WindowDrop)
			EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
	private:
		std::vector<std::filesystem::path> m_Paths;
	};
}