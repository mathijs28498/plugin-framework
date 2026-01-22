add_library(test_lib_i INTERFACE)

target_sources(test_lib_i
    INTERFACE
        FILE_SET HEADERS
        FILES
            test_lib_i.h
)