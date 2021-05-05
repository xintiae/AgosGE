cmake_minimum_required(VERSION 3.1.9)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


project(AgosGE VERSION 0.1 LANGUAGES CXX)

file(GLOB_RECURSE AGOSGE_CPPS examples/*.cpp examples/*/*.cpp)
set(EXECUTABLE_OUTPUT_PATH bin)

add_subdirectory(Agos)
add_executable(AgosGE ${AGOSGE_CPPS})

target_link_libraries(AgosGE PUBLIC Agos)
target_include_directories(AgosGE PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(AgosGE PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/Agos ${CMAKE_SOURCE_DIR}/Agos/vendor)
set_target_properties(AgosGE PROPERTIES LINKER_LANGUAGE CXX)


find_package(Spdlog REQUIRED)
target_include_directories(AgosGE PUBLIC ${SPDLOG_INCLUDE_DIR})
target_link_libraries(AgosGE PUBLIC Spdlog::Spdlog)

find_package(glfw3 3.3 REQUIRED)
target_include_directories(AgosGE PUBLIC glfw3::glfw)
target_link_libraries(AgosGE PUBLIC glfw)

find_package(Vulkan REQUIRED)
target_include_directories(AgosGE PUBLIC ${VULKAN_INCLUDE_DIRS})
target_link_libraries(AgosGE PUBLIC Vulkan::Vulkan)