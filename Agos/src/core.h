#ifndef AG_CORE_H__
#define AG_CORE_H__

#include "Agos/src/base.h"
#include <cstdint>

// - - - - - - - - - - - - - - - - - - - VERSION - - - - - - - - - - - - - - - - - - -
#define AG_MAKE_VERSION(major, minor, patch)\
    ((uint32_t)(major << 28) | (uint32_t)(minor << 14) | (uint32_t)(patch << 0))
#define AG_CURRENT_VERSION      AG_MAKE_VERSION(1, 0, 0)
#define AG_AGS_MODEL_VERSION    1
// - - - - - - - - - - - - - - - - - - - VERSION - - - - - - - - - - - - - - - - - - -



// - - - - - - - - - - - - - - - - - - - WINDOW - - - - - - - - - - - - - - - - - - - -

// window's sizes
#define AG_DEFAUT_WINDOW_WIDTH 800
#define AG_DEFAUT_WINDOW_HEIGHT 600

#define AG_MIN_WINDOW_WIDTH 100
#define AG_MIN_WINDOW_HEIGHT 75

#define AG_MAX_WINDOW_WIDTH 1920
#define AG_MAX_WINDOW_HEIGHT 983


// graphics pipeline
#define AG_DEFAULT_VERTEX_SHADER_FOLDER     "Agos_vertex_shader"
#define AG_DEFAULT_FRAGMENT_SHADER_FOLDER   "Agos_fragment_shader"
#define AG_SHADERS_SHALL_COMPILE true

#define AG_DEFAULT_TEXTURE                  "/primitives/no_textures.png"
#define AG_DEFAULT_TEXTURE_OPACITY_NULL     "/primitives/no_textures_opacity_null.png"


// debug layers
#define AG_DEBUG_LAYERS_ENABLED
#define AG_ENABLE_DEBUG_VALIDATION_LAYER true


// Vulkan ===========
#define AG_SWAPCHAIN_RECREATION_LOCK_TIME_MS 500
#define AG_VULKAN_MAX_FRAMES_IN_FLIGHT 1
#define AG_VULKAN_DESCRIPTOR_POOL_MAX_SETS 500


// OpenGL ===========

// - - - - - - - - - - - - - - - - - - - WINDOW - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - MISC - - - - - - - - - - - - - - - - - - - -
#define AG_MARK_AS_USED(X)  ( (void*)(&(X)) )

// AgosGE's Logger
//                                -- msg severity | info | warn | error | critical
// #define AG_LOGGER_ALL          // allowed msgs | yes  | yes  |  yes  |   yes
#define AG_LOGGER_NO_INFO      // allowed msgs |  no  | yes  |  yes  |   yes
// #define AG_LOGGER_NO_WARN      // allowed msgs |  no  |  no  |  yes  |   yes
// #define AG_LOGGER_NO_LOGGER    // allowed msgs |  no  |  no  |   no  |    no
// - - - - - - - - - - - - - - - - - - - MISC - - - - - - - - - - - - - - - - - - - -


namespace Agos
{
enum AgResult : int8_t
{
// misc =============
    AG_MIN_ENUM                                     = -2,
    AG_ERROR_CODE_UNDEFINED                         = -1,
    AG_SUCCESS  = 0,
    AG_FAILED   = 1,
    AG_INSTANCE_ALREADY_TERMINATED                  = 2,

// glfw =============
    AG_FAILED_TO_CREATE_GLFW_INSTANCE               = 9,

// Vulkan ===========
    AG_FAILED_TO_CREATE_VULKAN_INSTANCE             = 10,
    AG_VALIDATION_LAYERS_NOT_AVAILABLE              = 11,
    AG_FAILED_TO_SET_UP_VULKAN_DEBUG_MESSENGER      = 12,
    AG_FAILED_TO_CREATE_WINDOW_SURFACE              = 13,
    AG_NO_VULKAN_COMPATIBLE_GPU_FOUND               = 14,
    AG_FAILED_TO_FIND_SUITABLE_GPU                  = 15,
    AG_FAILED_TO_CREATE_LOGICAL_DEVICE_FROM_GPU     = 16,
    AG_FAILED_TO_CREATE_SWAPCHAIN                   = 17,
    AG_FAILED_TO_CREATE_RENDER_PASS                 = 18,
    AG_FAILED_TO_CREATE_DESCRIPTOR_SET_LAYOUT       = 19,
    AG_FAILED_TO_CREATE_DESCRIPTOR_POOL             = 20,
    AG_FAILED_TO_ALLOCATE_DESCRIPTOR_SETS           = 21,
    AG_FAILED_TO_CREATE_GRAPHICS_PIPELINE_LAYOUT    = 22,
    AG_FAILED_TO_CREATE_GRAPHICS_PIPELINE           = 23,
    AG_FAILED_TO_CREATE_FRAMEBUFFERS                = 24,
    AG_FAILED_TO_CLEAR_COMPILED_SHADER              = 25,
    AG_FAILED_TO_CREATE_TEXTURE_SAMPLER             = 26,
    AG_VERTEX_BUFFER_ALREADY_FREED                  = 27,
    AG_INDEX_BUFFER_ALREADY_FREED                   = 28,
    AG_UNIFORM_BUFFERS_ALREADY_FREED                = 29,
    AG_FAILED_TO_CREATE_COMMAND_POOL                = 30,
    AG_COMMAND_BUFFERS_ALREADY_FREED                = 31,
    AG_SEMAPHORES_FENCES_ALREADY_TERMINATED         = 32,
    AG_RECREATED_SWAPCHAIN                          = 33,
    AG_UPDATE_MODELS_DONT_MATCH                     = 34,

// tiny obj loader ==
    AG_FAILED_TO_READ_OBJ_FILE                      = 35,

// misc =============
    AG_MAX_ENUM                                     = 127
};

typedef bool AgBool;

}   // namespace Agos

#endif
