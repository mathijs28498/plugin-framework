cmake_minimum_required(VERSION 3.23)

project(NotepadC C)

message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C compiler ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "C compiler version: ${CMAKE_C_COMPILER_VERSION}")

add_subdirectory(lib)
add_subdirectory(src)