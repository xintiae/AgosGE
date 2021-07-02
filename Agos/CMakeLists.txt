cmake_minimum_required(VERSION 3.1.9 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Agos VERSION 0.1 LANGUAGES CXX)

file(GLOB_RECURSE AGOS_CPPS ./src/*.cpp
                            ./src/*/*.cpp
                            ./src/*/*/*.cpp)

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

target_compile_definitions(Agos PUBLIC -DAG_SHADERS_PATH="${CMAKE_SOURCE_DIR}/Agos/shaders" -DAG_MODELS_PATH="${CMAKE_SOURCE_DIR}/Agos/models")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(Agos PROPERTIES COMPILE_FLAGS "-Wall -Wextra -fPIC -fvisibility=hidden")
endif()