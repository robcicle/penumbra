#include "ppch.h"
#include "core/log.h"

namespace penumbra 
{
	Ref<spdlog::logger> CLog::s_spCoreLogger;
	Ref<spdlog::logger> CLog::s_spClientLogger;

	void CLog::Init()
	{
		PENUMBRA_PROFILE_FUNC();

		std::vector<spdlog::sink_ptr> vecpLogSinks;
		vecpLogSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		vecpLogSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("penumbra.log", true));

		vecpLogSinks[0]->set_pattern(kLogSinkColorPattern);
		vecpLogSinks[1]->set_pattern(kLogSinkFilePattern);

		s_spCoreLogger = std::make_shared<spdlog::logger>(kCoreLoggerName, begin(vecpLogSinks), end(vecpLogSinks));
		spdlog::register_logger(s_spCoreLogger);
		s_spCoreLogger->set_level(spdlog::level::trace);
		s_spCoreLogger->flush_on(spdlog::level::trace);

		s_spClientLogger = std::make_shared<spdlog::logger>(kClientLoggerName, begin(vecpLogSinks), end(vecpLogSinks));
		spdlog::register_logger(s_spClientLogger);
		s_spClientLogger->set_level(spdlog::level::trace);
		s_spClientLogger->flush_on(spdlog::level::trace);
	}

}