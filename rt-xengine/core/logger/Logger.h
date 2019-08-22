#ifndef LOGGER_H
#define LOGGER_H

#include "spdlog/logger.h" 
// include for custom formats (override ToString(std::ostream& os) const from engine object)
#include "spdlog/fmt/ostr.h"

namespace Core
{
	class Log
	{
	public:
		static void Init(LogLevelTarget level);

		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_logger; }
	private:
		static std::shared_ptr<spdlog::logger> s_logger;
	};
}

#define RT_XENGINE_LOGGER_INIT(level) Core::Log::Init(level)

// force log by matching the log level of the logger - except when level is LL_OFF
#define RT_XENGINE_LOG_AT_LOWEST_LEVEL(...)     Core::Log::GetLogger()->log(Core::Log::GetLogger()->level(), __VA_ARGS__)

#define RT_XENGINE_LOG_TRACE(...)     Core::Log::GetLogger()->trace(__VA_ARGS__)
#define RT_XENGINE_LOG_DEBUG(...)     Core::Log::GetLogger()->debug(__VA_ARGS__)
#define RT_XENGINE_LOG_INFO(...)      Core::Log::GetLogger()->info(__VA_ARGS__)
#define RT_XENGINE_LOG_WARN(...)      Core::Log::GetLogger()->warn(__VA_ARGS__)
#define RT_XENGINE_LOG_ERROR(...)     Core::Log::GetLogger()->error(__VA_ARGS__)
#define RT_XENGINE_LOG_FATAL(...)     Core::Log::GetLogger()->critical(__VA_ARGS__)

#define RT_XENGINE_ASSERT(condition, ...) do { if(!(condition)) { RT_XENGINE_LOG_FATAL("Assertation failed: (" #condition "): " __VA_ARGS__ ); abort(); } } while(0)
#define RT_XENGINE_ASSERT_RETURN_FALSE(condition, ...) do { if(!(condition)) { RT_XENGINE_LOG_FATAL("Assertation failed: (" #condition "): " __VA_ARGS__ ); return false; } } while(0)

#endif // LOGGER_H
