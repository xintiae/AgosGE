#include "logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include "base.h"

std::shared_ptr<spdlog::logger> AG_API Agos::coreLogger;
std::shared_ptr<spdlog::logger> AG_API Agos::clientLogger;


#ifdef AG_DEBUG_LAYERS_ENABLED
Agos::AgResult Agos::init_loggers()
{
    spdlog::set_pattern("[%D - %T | %n in thread %t with process ID %P] [%^%l%$ in file %@] %v");
    coreLogger = std::shared_ptr<spdlog::logger>();
    clientLogger = std::shared_ptr<spdlog::logger>();

    coreLogger = spdlog::stdout_color_mt("AGOS");
    clientLogger = spdlog::stdout_color_mt("CLIENT");

    coreLogger->set_level(spdlog::level::info);
    clientLogger->set_level(spdlog::level::info);

    return Agos::AG_SUCCESS;
}

#else
Agos::AgResult Agos::init_loggers()
{
    spdlog::set_pattern("[%D - %T | %n] [%^%l%$] %v");
    coreLogger = spdlog::stdout_color_mt("AGOS");
    clientLogger = spdlog::stdout_color_mt("CLIENT");

    coreLogger->set_level(spdlog::level::warn);
    clientLogger->set_level(spdlog::level::warn);

    return Agos::AG_SUCCESS;
}
#endif