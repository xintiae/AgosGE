cmake_minimum_required(VERSION 3.1.9 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(AgosGE VERSION 0.1 LANGUAGES CXX)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    execute_process(
        COMMAND  bash ${CMAKE_SOURCE_DIR}/scripts/build_linux.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/scripts
    )
endif()

add_subdirectory(Agos)
add_subdirectory(${CMAKE_SOURCE_DIR}/Agos/vendor/EventBus/lib)

set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBRARY_OUTPUT_PATH lib)

file(GLOB_RECURSE AGOSGE_CPPS examples/*.cpp examples/*/*.cpp)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(Agos/vendor/GLFW)

foreach(EXECS ${AGOSGE_CPPS})

    # Not yet used because it doesn't remove the filename extension
    #if(${CMAKE_VERSION} VERSION_LESS "3.20.0") 
    #    message("Please consider to switch to CMake 3.20.0")
    #else()
    #    cmake_path(GET EXECS FILENAME EXEC_NAME)
    #endif()
    
    get_filename_component(EXEC_NAME "${EXECS}" NAME_WE)
    
    add_executable(${EXEC_NAME} ${EXECS})
    
    target_link_libraries(${EXEC_NAME} PRIVATE Agos)
    target_link_directories(${EXEC_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${PROJECT_BINARY_DIR}) 
    target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_include_directories(${EXEC_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/Agos ${CMAKE_SOURCE_DIR}/Agos/vendor ${PROJECT_BINARY_DIR})

    set_target_properties(${EXEC_NAME} PROPERTIES LINKER_LANGUAGE CXX)

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	    set_target_properties(${EXEC_NAME} PROPERTIES COMPILE_FLAGS "-Wall -Wextra -fPIC")

        find_package(glfw3 3.3 REQUIRED)
        target_include_directories(${EXEC_NAME} PRIVATE glfw3::glfw)
        target_link_libraries(${EXEC_NAME} PRIVATE glfw)
    else()
       
        set( ENKITS_BUILD_EXAMPLES OFF CACHE BOOL "Build basic example applications" )
        set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL  "GLFW lib only" )
        set( GLFW_BUILD_TESTS OFF CACHE BOOL  "GLFW lib only" )
        set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
        set( GLFW_BUILD_INSTALL OFF CACHE BOOL  "GLFW lib only" )

        include_directories("Agos/vendor/GLFW/include")
        target_link_libraries(${EXEC_NAME} PUBLIC glfw)
    endif()
    
    target_include_directories(${EXEC_NAME} PRIVATE OpenGL::GL)
    target_link_libraries(${EXEC_NAME} PRIVATE OpenGL::GL)

    target_include_directories(${EXEC_NAME} PRIVATE ${Vulkan_INCLUDE_DIR})
    target_link_libraries(${EXEC_NAME} PRIVATE ${Vulkan_LIBRARIES})

    target_link_libraries(${EXEC_NAME} PUBLIC Threads::Threads)

    target_link_libraries(${EXEC_NAME} PRIVATE Dexode::EventBus)

endforeach()
