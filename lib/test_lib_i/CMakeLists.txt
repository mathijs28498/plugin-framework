add_library(TestLibI INTERFACE)

target_sources(TestLibI
    INTERFACE
        FILE_SET HEADERS
        FILES
            test_lib_i.h
)