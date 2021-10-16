#pragma once

#include "Agos/src/base.h"

#define AG_MAKE_VERSION(major, minor, patch)\
    ((uint32_t)(major << 28) | (uint32_t)(minor << 14) | (uint32_t)(patch << 0))
#define AG_CURRENT_VERSION      AG_MAKE_VERSION(1, 0, 0)
#define AG_AGS_MODEL_VERSION    1


#define AG_DEFAUT_WINDOW_WIDTH 800
#define AG_DEFAUT_WINDOW_HEIGHT 600

#define AG_MAX_WINDOW_WIDTH 1920
#define AG_MAX_WINDOW_HEIGHT 983

#define AG_MARK_AS_USED(X)  ( (void*)(&(X)) )

#define AG_DEBUG_LAYERS_ENABLED
#define AG_ENABLE_DEBUG_VALIDATION_LAYER true

// #define AG_LOGGER_ALL
// #define AG_LOGGER_NO_INFO
#define AG_LOGGER_NO_WARN
// #define AG_LOGGER_NO_LOGGER

#define AG_VULKAN_MAX_FRAMES_IN_FLIGHT 2

#define AG_DEFAULT_VERTEX_SHADER_FOLDER "Agos_vertex_shader"
#define AG_DEFAULT_FRAGMENT_SHADER_FOLDER "Agos_fragment_shader"
#define AG_SHADERS_COMPILE_ANYWAY true

#define AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS 100

#define AG_DEFAULT_MODEL_TEXTURE               "/primitives/no_textures.png"
#define AG_DEFAULT_MODEL_TEXTURE_OPACITY_NULL  "/primitives/no_textures_opacity_null.png"

#define AG_DEFAULT_MODEL_NAME "Unnamed"

namespace Agos
{
enum AgResult {
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
    AG_FAILED_TO_CREATE_COMMAND_POOL                = 29,
    AG_COMMAND_BUFFERS_ALREADY_FREED                = 30,
    AG_SEMAPHORES_FENCES_ALREADY_TERMINATED         = 31,
    AG_RECREATED_SWAPCHAIN                          = 32,
    AG_UPDATE_MODELS_DONT_MATCH                     = 33,
    AG_FAILED_TO_READ_OBJ_FILE                      = 34
};

typedef bool AgBool;

}   // namespace Agos
