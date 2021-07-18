#pragma once

#include "Agos/src/base.h"

#define AG_WINDOW_WIDTH 800
#define AG_WINDOW_HEIGHT 600

#define AG_MARK_AS_USED(X)  ( (void*)(&(X)) )

namespace Agos
{
typedef enum AgResult {
    AG_SUCCESS  = 0,
    AG_FAILED   = 1,
    AG_INSTANCE_ALREADY_TERMINATED                  = 2,
    AG_FAILED_TO_CREATE_GLFW_INSTANCE               = 9,
    AG_FAILED_TO_CREATE_VULKAN_INSTANCE             = 10,
    AG_FAILED_TO_SET_UP_VULKAN_DEBUG_MESSENGER      = 11,
    AG_FAILED_TO_CREATE_WINDOW_SURFACE              = 12,
    AG_NO_VULKAN_COMPATIBLE_GPU_FOUND               = 13,
    AG_FAILED_TO_FIND_SUITABLE_GPU                  = 14,
    AG_FAILED_TO_CREATE_LOGICAL_DEVICE_FROM_GPU     = 15,
    AG_FAILED_TO_CREATE_SWAPCHAIN                   = 16,
    AG_FAILED_TO_CREATE_RENDER_PASS                 = 17,
    AG_FAILED_TO_CREATE_DESCRIPTOR_SET_LAYOUT       = 18,
    AG_FAILED_TO_CREATE_DESCRIPTOR_POOL             = 19,
    AG_FAILED_TO_ALLOCATE_DESCRIPTOR_SETS           = 20,
    AG_FAILED_TO_CREATE_GRAPHICS_PIPELINE_LAYOUT    = 21,
    AG_FAILED_TO_CREATE_GRAPHICS_PIPELINE           = 22,
    AG_FAILED_TO_CREATE_FRAMEBUFFERS                = 23,
    AG_FAILED_TO_CLEAR_COMPILED_SHADER              = 24,
    AG_FAILED_TO_CREATE_TEXTURE_SAMPLER             = 25,
    AG_VERTEX_BUFFER_ALREADY_FREED                  = 26,
    AG_INDEX_BUFFER_ALREADY_FREED                   = 27,
    AG_UNIFORM_BUFFERS_ALREADY_FREED                = 28,
    AG_COMMAND_BUFFERS_ALREADY_FREED                = 29,
    AG_SEMAPHORES_FENCES_ALREADY_TERMINATED         = 30,
    AG_RECREATED_SWAPCHAIN                          = 31
} AgResult;

}   // namespace Agos
