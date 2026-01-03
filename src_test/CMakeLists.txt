add_executable(MyProgram main.c)

target_compile_features(MyProgram PUBLIC c_std_99)

set_target_properties(MyProgram
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_link_libraries(MyProgram
    PRIVATE
        TestLib
)

target_link_libraries(MyProgram
    PRIVATE
        TestLibI
)