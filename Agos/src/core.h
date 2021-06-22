#pragma once

#define AG_WINDOW_WIDTH 800
#define AG_WINDOW_HEIGHT 600

namespace Agos
{

typedef enum AgResult {
    AG_SUCCESS  = 0,
    AG_FAILED   = 1,
    AG_INSTANCE_ALREADY_TERMINATED              = 2,
    AG_FAILED_TO_CREATE_GLFW_INSTANCE           = 9,
    AG_FAILED_TO_CREATE_VULKAN_INSTANCE         = 10,
    AG_FAILED_TO_SET_UP_VULKAN_DEBUG_MESSENGER  = 11,
    AG_FAILED_TO_CREATE_WINDOW_SURFACE          = 12,
    AG_NO_VULKAN_COMPATIBLE_GPU_FOUND           = 13,
    AG_FAILED_TO_FIND_SUITABLE_GPU              = 14,
    AG_FAILED_TO_CREATE_LOGICAL_DEVICE_FROM_GPU = 15,
    AG_FAILED_TO_CREATE_SWAPCHAIN               = 16
} AgResult;

}   // namespace Agos
