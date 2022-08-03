cmake_minimum_required(VERSION 3.1.9 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# On which graphics API shall AgosGE's renderer be based on?
set(AGOS_GRAPHICS_API "Vulkan" CACHE STRING
    "Specifies which graphics API AgosGE's Renderer shall use when running (current renderer's version supports either OpenGL or Vulkan).
    In order to select an API or another, simply specify either 'Vulkan' or 'OpenGL'.
    Default API is Vulkan :P.")

# do you want to see about 100 dumb warnings? -thx to EventBus
set(AGOS_GIMME_SOME_WARNINGS OFF)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


project(AgosGE VERSION 0.1 LANGUAGES CXX)


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    execute_process(
        COMMAND  bash ${CMAKE_SOURCE_DIR}/scripts/build_linux.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/scripts
    )
endif()
add_subdirectory(${CMAKE_SOURCE_DIR}/Agos/vendor/EventBus/lib)


set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBRARY_OUTPUT_PATH lib)


file(GLOB_RECURSE AGOS_CPPS
    ${CMAKE_SOURCE_DIR}/Agos/agos.cpp
    ${CMAKE_SOURCE_DIR}/Agos/src/*.cpp
    ${CMAKE_SOURCE_DIR}/Agos/src/*/*.cpp
    ${CMAKE_SOURCE_DIR}/Agos/src/*/*/*.cpp
    ${CMAKE_SOURCE_DIR}/Agos/src/*/*/*/*.cpp
    ${CMAKE_SOURCE_DIR}/Agos/src/*/*/*/*/*.cpp
)
add_executable(AgosGE "${AGOS_CPPS}")
    
target_link_directories     (AgosGE PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/Agos
    ${CMAKE_SOURCE_DIR}/Agos/src
    ${CMAKE_SOURCE_DIR}/Agos/vendor
    ${CMAKE_SOURCE_DIR}/Agos/vendor/GLFW/include
    ${CMAKE_SOURCE_DIR}/Agos/vendor/spdlog/include
    ${CMAKE_SOURCE_DIR}/Agos/vendor/EventBus/lib/src
    ${Vulkan_INCLUDE_DIR}
    ${PROJECT_BINARY_DIR}
) 
target_include_directories  (AgosGE PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Agos
    ${CMAKE_SOURCE_DIR}/Agos/src
    ${CMAKE_SOURCE_DIR}/Agos/vendor
    ${CMAKE_SOURCE_DIR}/Agos/vendor/GLFW/include
    ${CMAKE_SOURCE_DIR}/Agos/vendor/spdlog/include
    ${CMAKE_SOURCE_DIR}/Agos/vendor/EventBus/lib/src
    ${CMAKE_CURRENT_LIST_DIR}
    ${Vulkan_INCLUDE_DIR}
)
set_target_properties       (AgosGE PROPERTIES  LINKER_LANGUAGE CXX)

target_compile_definitions(AgosGE PUBLIC 
    AG_SOURCE_PATH="${CMAKE_SOURCE_DIR}"
    AG_EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples"
    AG_SHADERS_PATH="${CMAKE_SOURCE_DIR}/Agos/shaders"
)
if ("${AGOS_GRAPHICS_API}" STREQUAL "Vulkan")
    target_compile_definitions(AgosGE PUBLIC AG_GRAPHICS_API_VULKAN=1)
    message(STATUS "Specified Vulkan-based renderer")

elseif("${AGOS_GRAPHICS_API}" STREQUAL "OpenGL")
    target_compile_definitions(AgosGE PUBLIC AG_GRAPHICS_API_OPENGL=1)
    message(STATUS "Specified OpenGL-based renderer")

else()
    message(FATAL_ERROR "Current graphics API specified for rendering op is urecognized! Did you specified it using -DAGOS_GRAPHICS_API?")
endif()


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set_target_properties(AgosGE PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -fPIC")
    if (AGOS_GIMME_SOME_WARNINGS)
        set_target_properties(AgosGE PROPERTIES COMPILE_FLAGS "-fPIC -fvisibility=hidden")
    else ()
        set_target_properties(AgosGE PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -fPIC -fvisibility=hidden")
    endif()

    find_package(glfw3 3.3 REQUIRED)
    target_include_directories(AgosGE PUBLIC glfw3::glfw)
    target_link_libraries(AgosGE PUBLIC glfw)

else()
    add_subdirectory(Agos/vendor/GLFW)
    set( ENKITS_BUILD_EXAMPLES  OFF CACHE BOOL "Build basic example applications" )
    set( GLFW_BUILD_EXAMPLES    OFF CACHE BOOL  "GLFW lib only" )
    set( GLFW_BUILD_TESTS       OFF CACHE BOOL  "GLFW lib only" )
    set( GLFW_BUILD_DOCS        OFF CACHE BOOL  "GLFW lib only" )
    set( GLFW_BUILD_INSTALL     OFF CACHE BOOL  "GLFW lib only" )

    include_directories("Agos/vendor/GLFW/include")
    target_link_libraries(AgosGE PUBLIC glfw)
endif()


find_package(OpenGL REQUIRED)
target_include_directories(AgosGE PUBLIC OpenGL::GL)
target_link_libraries(AgosGE PUBLIC OpenGL::GL)

find_package(Vulkan REQUIRED)
target_include_directories(AgosGE PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(AgosGE PUBLIC ${Vulkan_LIBRARIES})

find_package(Threads REQUIRED)
target_link_libraries(AgosGE PUBLIC Threads::Threads Dexode::EventBus ${CMAKE_DL_LIBS})
