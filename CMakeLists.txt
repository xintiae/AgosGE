cmake_minimum_required(VERSION 3.1.9 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(Agos)
add_subdirectory(${CMAKE_SOURCE_DIR}/Agos/vendor/EventBus/lib)
add_subdirectory(${CMAKE_SOURCE_DIR}/Agos/vendor/GLFW)

set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBRARY_OUTPUT_PATH lib)

project(AgosGE VERSION 0.1 LANGUAGES CXX)

file(GLOB_RECURSE AGOSGE_CPPS examples/*.cpp examples/*/*.cpp)

add_executable(AgosGE "${AGOSGE_CPPS}")

target_link_libraries(AgosGE PRIVATE Agos)
target_link_directories(AgosGE PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${PROJECT_BINARY_DIR}) 
target_include_directories(AgosGE PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(AgosGE PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/Agos ${CMAKE_SOURCE_DIR}/Agos/vendor ${PROJECT_BINARY_DIR})

set_target_properties(AgosGE PROPERTIES LINKER_LANGUAGE CXX)

if(NOT WIN32)
	set_target_properties(AgosGE PROPERTIES COMPILE_FLAGS "-Wall -Wextra -fPIC")

	find_package(glfw3 3.3 REQUIRED)
	target_include_directories(AgosGE PUBLIC glfw3::glfw)
	target_link_libraries(AgosGE PUBLIC glfw)
else()
	include(FindVulkan)
	target_include_directories(AgosGE PUBLIC glfw3::glfw "${CMAKE_SOURCE_DIR}/Agos/vendor/GLFW/include")
	target_link_libraries(AgosGE PUBLIC glfw "${CMAKE_SOURCE_DIR}/Agos/vendor/lib/GLFW/glfw3.lib")
endif()

find_package(OpenGL REQUIRED)
target_include_directories(AgosGE PRIVATE OpenGL::GL)
target_link_libraries(AgosGE PRIVATE OpenGL::GL)

find_package(Vulkan REQUIRED)
target_include_directories(AgosGE PRIVATE ${Vulkan_INCLUDE_DIR})
target_link_libraries(AgosGE PRIVATE ${Vulkan_LIBRARIES})

find_package(Threads REQUIRED)
target_link_libraries(AgosGE PUBLIC Threads::Threads)

target_link_libraries(AgosGE PRIVATE Dexode::EventBus)