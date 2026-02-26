# TODO: Add error handling to all cmake stuff
cmake_minimum_required(VERSION 3.23)

project(NotepadC C)

message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C compiler ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "C compiler version: ${CMAKE_C_COMPILER_VERSION}")

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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

configure_file(
    "${CMAKE_SOURCE_DIR}/plugin_registry.json"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugin_registry.json" 
    COPYONLY
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/lib/plugin_manager")

add_subdirectory(lib)
add_subdirectory(plugins)
add_subdirectory(src)
add_subdirectory(plugin_manager_include)