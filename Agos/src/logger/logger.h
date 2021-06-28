#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include AG_SPDLOG_INCLUDE
#include <memory>

namespace Agos
{
extern AG_API std::shared_ptr<spdlog::logger> AgCoreLogger;
extern AG_API std::shared_ptr<spdlog::logger> AgClientLogger;

AgResult ag_init_loggers();
};

#define AG_CORE_INFO(...)       Agos::AgCoreLogger->info(__VA_ARGS__)
#define AG_CORE_WARN(...)       Agos::AgCoreLogger->warn(__VA_ARGS__)
#define AG_CORE_ERROR(...)      Agos::AgCoreLogger->error(__VA_ARGS__)
#define AG_CORE_CRITICAL(...)   Agos::AgCoreLogger->critical(__VA_ARGS__)

#define AG_CLIENT_INFO(...)     Agos::AgClientLogger->info(__VA_ARGS__)
#define AG_CLIENT_WARN(...)     Agos::AgClientLogger->warn(__VA_ARGS__)
#define AG_CLIENT_ERROR(...)    Agos::AgClientLogger->error(__VA_ARGS__)
#define AG_CLIENT_CRITICAL(...) Agos::AgClientLogger->critical(__VA_ARGS__)
