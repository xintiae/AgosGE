#include "logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include "base.h"

std::shared_ptr<spdlog::logger> AG_API Agos::AgCoreLogger;
std::shared_ptr<spdlog::logger> AG_API Agos::AgClientLogger;

#ifdef AG_DEBUG_LAYERS_ENABLED
Agos::AgResult Agos::ag_init_loggers()
{
    // likes talking ?
    // spdlog::set_pattern("[%D - %T | %n in thread %t with process ID %P] [%^%l%$ in file %@] %v");

    // prefer calm and quietness?
    spdlog::set_pattern("[%D - %T | %n] [%^%l%$] %v");

    AgCoreLogger = std::shared_ptr<spdlog::logger>();
    AgClientLogger = std::shared_ptr<spdlog::logger>();

    AgCoreLogger = spdlog::stdout_color_mt("AgCore");
    AgClientLogger = spdlog::stdout_color_mt("Client");

    AgCoreLogger->set_level(spdlog::level::info);
    AgClientLogger->set_level(spdlog::level::info);

    return Agos::AG_SUCCESS;
}

#else
Agos::AgResult Agos::ag_init_loggers()
{
    spdlog::set_pattern("[%D - %T | %n] [%^%l%$] %v");
    AgCoreLogger = spdlog::stdout_color_mt("AGOS");
    AgClientLogger = spdlog::stdout_color_mt("CLIENT");

    AgCoreLogger->set_level(spdlog::level::warn);
    AgClientLogger->set_level(spdlog::level::warn);

    return Agos::AG_SUCCESS;
}
#endif