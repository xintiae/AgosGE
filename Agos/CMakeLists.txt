cmake_minimum_required(VERSION 3.1.9 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Agos VERSION 0.1 LANGUAGES CXX)
file(GLOB_RECURSE AGOS_CPPS ./src/*.cpp
                            ./src/*/*.cpp
                            ./src/*/*/*.cpp)

add_library(Agos SHARED ${AGOS_CPPS})
target_include_directories(Agos PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/Agos/ ${CMAKE_SOURCE_DIR}/Agos/src)
set_target_properties(Agos PROPERTIES COMPILE_FLAGS "-Wall -Wextra -fpic -fvisibility=hidden -lstdc++fs")