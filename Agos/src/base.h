#pragma once
/**
 * @file ~/Agos/src/base.h
 * @brief base file for any Agos members
 * Contains any required Agos Api members macros
*/


#define AG_DEBUG_LAYERS_ENABLED
#define AG_ENABLE_DEBUG_VALIDATION_LAYER true

#define AG_DEFAULT_VERTEX_SHADER_FOLDER "Agos_vertex_shader"
#define AG_DEFAULT_FRAGMENT_SHADER_FOLDER "Agos_fragment_shader"
#define AG_SHADERS_COMPILE_ANYWAY true

// #define AGOS_COMPILED_STATIC
#define AGOS_COMPILED_SHARED

#define AGOS_SYMBOLS_EXPORT
// #define AGOS_SYMBOLS_IMPORT

#if defined _WIN32 || defined __CYGWIN__
  #define AG_GLFW_INCLUDE             "GLFW/glfw3.h"
  #define AG_VULKAN_INCLUDE           "vulkan/vulkan.h"
  #define AG_SPDLOG_INCLUDE           "spdlog/spdlog.h"
  #define AG_EVENTBUS_INCLUDE         "dexode/EventBus.hpp"

  #define GLM_FORCE_RADIANS
  #define GLM_FORCE_DEPTH_ZERO_TO_ONE
  #define GLM_ENABLE_EXPERIMENTAL
  #define AG_GLM_INCLUDE              "glm/glm/glm.hpp"

  #define TINYOBJLOADER_IMPLEMENTATION
  #define AG_TINY_OBJ_LOADER_INCLUDE  "tiny_obj_loader/tiny_obj_loader.h"

  #define STB_IMAGE_IMPLEMENTATION
  #define AG_STB_INCLUDE              "stb/stb.h"

  #define AG_JSON_INCLUDE             "nlohmann/json.hpp"

  #define AG_SHADER_SCRIPT_COMPILE_EXTENTION  ".bat"
  #define AG_SHADER_DELETE_COMPILED_SHADER_COMMAND "del"

  #define AGOS_HELPER_SHARED_IMPORT __declspec(dllimport)
  #define AGOS_HELPER_SHARED_EXPORT __declspec(dllexport)
  #define AGOS_HELPER_SHARED_LOCAL
#else
  #define AG_GLFW_INCLUDE             "GLFW/glfw3.h"
  #define AG_VULKAN_INCLUDE           "vulkan/vulkan.h"
  #define AG_SPDLOG_INCLUDE           "spdlog/spdlog.h"
  #define AG_EVENTBUS_INCLUDE         "Agos/vendor/EventBus/lib/src/dexode/EventBus.hpp"
 
  #define GLM_FORCE_RADIANS
  #define GLM_FORCE_DEPTH_ZERO_TO_ONE
  #define GLM_ENABLE_EXPERIMENTAL
  #define AG_GLM_INCLUDE              "glm/glm.hpp"
 
  #define TINYOBJLOADER_IMPLEMENTATION
  #define AG_TINY_OBJ_LOADER_INCLUDE  "tiny_obj_loader.h"
 
  #define STB_IMAGE_IMPLEMENTATION
  #define AG_STB_INCLUDE              "stb/stb_image.h"
 
  #define AG_JSON_INCLUDE             "nlohmann/json.hpp"

  #define AG_SHADER_SCRIPT_COMPILE_EXTENTION  ".sh"
  #define AG_SHADER_DELETE_COMPILED_SHADER_COMMAND "rm -f"

  #if __GNUC__ >= 4
    #define AGOS_HELPER_SHARED_IMPORT __attribute__( (visibility ("default")) )
    #define AGOS_HELPER_SHARED_EXPORT __attribute__( (visibility ("default")) )
    #define AGOS_HELPER_SHARED_LOCAL  __attribute__( (visibility ("hidden"))  )
  #else
    #define AGOS_HELPER_SHARED_IMPORT
    #define AGOS_HELPER_SHARED_EXPORT
    #define AGOS_HELPER_SHARED_LOCAL
  #endif
#endif

// Agos target is SHARED (.dll / .so)
#ifdef AGOS_COMPILED_SHARED
  // local refers to non relative Agos symbols
  #define AG_LOCAL AGOS_HELPER_SHARED_LOCAL

  // export symbols
  #ifdef AGOS_SYMBOLS_EXPORT
    #define AG_API AGOS_HELPER_SHARED_EXPORT

  // import symbols
  #else
    #define AG_API AGOS_HELPER_SHARED_IMPORT

  #endif

// Agos target is STATIC (.lib / .a)
#else
  #define AG_API
  #define AG_LOCAL
#endif
