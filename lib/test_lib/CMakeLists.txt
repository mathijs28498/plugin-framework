
add_library(TestLib)

target_sources(TestLib
    PRIVATE
        test_lib.c

    PUBLIC
        FILE_SET HEADERS
        FILES
            test_lib.h
)