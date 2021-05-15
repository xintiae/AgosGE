#pragma once

#include <spdlog/spdlog.h>
#include <memory>
#include "core.h"
#include "base.h"

namespace Agos
{
extern AG_API std::shared_ptr<spdlog::logger> coreLogger;
extern AG_API std::shared_ptr<spdlog::logger> clientLogger;

AgResult init_loggers();
};

#define AG_CORE_INFO(...)       Agos::coreLogger->info(__VA_ARGS__)
#define AG_CORE_WARN(...)       Agos::coreLogger->warn(__VA_ARGS__)
#define AG_CORE_ERROR(...)      Agos::coreLogger->error(__VA_ARGS__)
#define AG_CORE_CRITICAL(...)   Agos::coreLogger->critical(__VA_ARGS__)

#define AG_CLIENT_INFO(...)     Agos::clientLogger->info(__VA_ARGS__)
#define AG_CLIENT_WARN(...)     Agos::clientLogger->warn(__VA_ARGS__)
#define AG_CLIENT_ERROR(...)    Agos::clientLogger->error(__VA_ARGS__)
#define AG_CLIENT_CRITICAL(...) Agos::clientLogger->critical(__VA_ARGS__)
