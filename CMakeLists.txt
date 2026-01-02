cmake_minimum_required(VERSION 3.23)

project(NotepadC C)

add_executable(MyProgram)

target_sources(MyProgram
    PRIVATE
        src_test/main.c
)

add_subdirectory(lib)

target_link_libraries(MyProgram
    PRIVATE
        TestLib
)

target_link_libraries(MyProgram
    PRIVATE
        TestLibI
)