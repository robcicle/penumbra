#pragma once

#include "core/base.h"

namespace spdlog
{
	class logger;
}

namespace penumbra
{
	class CLog
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetCoreLogger() { return s_spCoreLogger; }
		static Ref<spdlog::logger>& GetClientLogger() { return s_spClientLogger; }
	private:
		static Ref<spdlog::logger> s_spCoreLogger;
		static Ref<spdlog::logger> s_spClientLogger;
	};

}

// Core log macros
#define PENUMBRA_CORE_TRACE(...)    ::penumbra::CLog::GetCoreLogger()->trace(__VA_ARGS__)
#define PENUMBRA_CORE_INFO(...)     ::penumbra::CLog::GetCoreLogger()->info(__VA_ARGS__)
#define PENUMBRA_CORE_WARN(...)     ::penumbra::CLog::GetCoreLogger()->warn(__VA_ARGS__)
#define PENUMBRA_CORE_ERROR(...)    ::penumbra::CLog::GetCoreLogger()->error(__VA_ARGS__)
#define PENUMBRA_CORE_CRITICAL(...) ::penumbra::CLog::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define PENUMBRA_TRACE(...)	      ::penumbra::CLog::GetClientLogger()->trace(__VA_ARGS__)
#define PENUMBRA_INFO(...)	      ::penumbra::CLog::GetClientLogger()->info(__VA_ARGS__)
#define PENUMBRA_WARN(...)	      ::penumbra::CLog::GetClientLogger()->warn(__VA_ARGS__)
#define PENUMBRA_ERROR(...)	      ::penumbra::CLog::GetClientLogger()->error(__VA_ARGS__)
#define PENUMBRA_CRITICAL(...)	  ::penumbra::CLog::GetClientLogger()->critical(__VA_ARGS__)