cmake_minimum_required(VERSION 3.23)

project(NotepadC C)

message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C compiler ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "C compiler version: ${CMAKE_C_COMPILER_VERSION}")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(
        /W4       # Warning level 4 (Production quality - standard "strict")
        /WX       # Treat warnings as errors
        /permissive- # Enforce strict standard conformance (no MSVC extensions/loose rules)
    )
    add_compile_options(/Zc:preprocessor)
    
else()
    # GCC / Clang
    target_compile_options(Program PRIVATE 
        -Wall -Wextra -Werror 
    )
   
endif()

option(PLUGIN_MANAGER_GUI "Build plugin manager as a GUI" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/lib/plugin_manager_bootloader")

add_subdirectory(lib)
add_subdirectory(plugins)
add_subdirectory(src)
add_subdirectory(plugin_manager_include)