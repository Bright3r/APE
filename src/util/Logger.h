#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <memory>

namespace APE {

class Logger {
private:
	inline static std::shared_ptr<spdlog::logger> s_core_logger;

public:
	static void init() noexcept
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_core_logger = spdlog::stdout_color_mt("APE");
		s_core_logger->set_level(spdlog::level::trace);
	}

	[[nodiscard]] static std::shared_ptr<spdlog::logger> getCoreLogger() noexcept
	{
		return s_core_logger;
	}
};

// core logger macros
#define APE_TRACE(...)    ::APE::Logger::getCoreLogger()->trace(__VA_ARGS__)
#define APE_INFO(...)     ::APE::Logger::getCoreLogger()->info(__VA_ARGS__)
#define APE_WARN(...)     ::APE::Logger::getCoreLogger()->warn(__VA_ARGS__)
#define APE_ERROR(...)    ::APE::Logger::getCoreLogger()->error(__VA_ARGS__)
#define APE_FATAL(...)    ::APE::Logger::getCoreLogger()->critical(__VA_ARGS__)
#define APE_ABORT(...) {	\
	::APE::Logger::getCoreLogger()->critical(__VA_ARGS__);	\
	std::abort();	\
}
#define APE_CHECK(boolVal, ...) if (!boolVal) APE_ABORT(__VA_ARGS__);

};	// end of namespace APE
