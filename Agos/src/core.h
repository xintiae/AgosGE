#pragma once

namespace Agos
{

typedef enum AgResult {
    AG_SUCCESS = 0,
    AG_FAILED = 1,
    AG_FAILED_TO_CREATE_VULKAN_INSTANCE = 10,
    AG_FAILED_TO_SET_UP_VULKAN_DEBUG_MESSENGER = 11
} AgResult;

}   // namespace Agos
