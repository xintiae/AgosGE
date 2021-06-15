cmake_minimum_required(VERSION 3.1.9 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
#add_subdirectory(${CMAKE_SOURCE_DIR}/Agos/vendor/EventBus/lib)

set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBRARY_OUTPUT_PATH lib)

project(AgosGE VERSION 0.1 LANGUAGES CXX)

file(GLOB_RECURSE AGOSGE_CPPS examples/*.cpp examples/*/*.cpp)

add_subdirectory(Agos)
add_executable(AgosGE ${AGOSGE_CPPS})


target_link_libraries(AgosGE PUBLIC Agos)
target_link_directories(AgosGE PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${PROJECT_BINARY_DIR}) 
target_include_directories(AgosGE PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(AgosGE PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/Agos ${CMAKE_SOURCE_DIR}/Agos/vendor ${PROJECT_BINARY_DIR})

set_target_properties(AgosGE PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(AgosGE PROPERTIES COMPILE_FLAGS "-Wall -Wextra -fPIC -lstdc++fs")
set_target_properties(AgosGE PROPERTIES LINK_FLAGS "-Wall -Wextra -fPIC -lstdc++fs")


find_package(Spdlog REQUIRED)
target_include_directories(AgosGE PUBLIC ${SPDLOG_INCLUDE_DIR})
target_link_libraries(AgosGE PUBLIC Spdlog::Spdlog)

find_package(glfw3 3.3 REQUIRED)
target_include_directories(AgosGE PUBLIC glfw3::glfw)
target_link_libraries(AgosGE PUBLIC glfw)

find_package(Vulkan REQUIRED)
target_include_directories(AgosGE PUBLIC ${VULKAN_INCLUDE_DIRS})
target_link_libraries(AgosGE PUBLIC Vulkan::Vulkan)

find_package(Threads REQUIRED)
target_link_libraries(AgosGE PUBLIC Threads::Threads)

target_link_libraries(AgosGE PUBLIC Dexode::EventBus)
target_link_libraries(AgosGE PUBLIC stdc++fs)