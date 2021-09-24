cmake_minimum_required(VERSION 3.1.9 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# On which graphics API shall Agos's renderer be based on?
set(AGOS_GRAPHICS_API "Vulkan" CACHE STRING
    "Specifies which graphics API Agos's Renderer shall use when running (current renderer's version supports either OpenGL or Vulkan).
    In order to select an API or another, simply specify either 'Vulkan' or 'OpenGL'.
    Default API is Vulkan :P.")

# do you want to see about 100 dumb warnings? -thx to EventBus
set(AGOS_GIMME_SOME_WARNINGS OFF)


project(Agos VERSION 0.1 LANGUAGES CXX)
file(GLOB_RECURSE AGOS_CPPS ./src/*.cpp
                            ./src/*/*.cpp
                            ./src/*/*/*.cpp
                            ./src/*/*/*/*.cpp)
include(FindVulkan)

add_library(Agos STATIC ${AGOS_CPPS})
target_include_directories(Agos PUBLIC  ${CMAKE_SOURCE_DIR}
                                        ${CMAKE_SOURCE_DIR}/Agos
                                        ${CMAKE_SOURCE_DIR}/Agos/src
                                        ${CMAKE_SOURCE_DIR}/agos/vendor
                                        ${CMAKE_SOURCE_DIR}/Agos/vendor/GLFW/include
                                        ${CMAKE_SOURCE_DIR}/Agos/vendor/spdlog/include
                                        ${CMAKE_SOURCE_DIR}/Agos/vendor/EventBus/lib/src
                                        ${Vulkan_INCLUDE_DIR})

target_compile_definitions(Agos PUBLIC AG_SHADERS_PATH="${CMAKE_SOURCE_DIR}/Agos/shaders" AG_MODELS_PATH="${CMAKE_SOURCE_DIR}/Agos/models" AG_TEXTURES_PATH="${CMAKE_SOURCE_DIR}/Agos/textures")

if ("${AGOS_GRAPHICS_API}" STREQUAL "Vulkan")
    add_compile_definitions(AgosRenderer AG_GRAPHICS_API_VULKAN)
    message(STATUS "Specified Vulkan-based renderer")

elseif("${AGOS_GRAPHICS_API}" STREQUAL "OpenGL")
    add_compile_definitions(AgosRenderer AG_GRAPHICS_API_OPENGL)
    message(STATUS "Specified OpenGL-based renderer")

else()
    message(FATAL_ERROR "Current graphics API specified for rendering op is urecognized! Did you specified it using -DAGOS_GRAPHICS_API?")
endif()


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (AGOS_GIMME_SOME_WARNINGS)
        set_target_properties(Agos PROPERTIES COMPILE_FLAGS "-fPIC -fvisibility=hidden")
    else ()
        set_target_properties(Agos PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -fPIC -fvisibility=hidden")
    endif()

endif()
