add_library(test_yes test_yes.c)

target_sources(test_yes
    PUBLIC
        FILE_SET HEADERS
        FILES
            test_yes.h
)