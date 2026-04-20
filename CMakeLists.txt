cmake_minimum_required(VERSION 3.23)

project(NotepadC LANGUAGES C CXX)

message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C compiler ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "C compiler version: ${CMAKE_C_COMPILER_VERSION}")


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # Don't use add_compile_options() globally if using 3rd party libs
    # Apply these to a custom variable or target instead
    set(MY_STRICT_WARNINGS /W4 /WX /permissive- /Zc:preprocessor)
else()
    set(MY_STRICT_WARNINGS -Wall -Wextra -Werror)
endif()

option(PLUGIN_MANAGER_GUI "Build plugin manager as a GUI" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/plugin_sdk/build_tools/cmake")

add_subdirectory(lib)
add_subdirectory(plugins)
add_subdirectory(src)
add_subdirectory(plugin_sdk)
add_subdirectory(plugin_manager_bootloader)