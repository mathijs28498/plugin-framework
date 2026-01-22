
add_library(test_lib)

target_sources(test_lib
    PRIVATE
        test_lib.c

    PUBLIC
        FILE_SET HEADERS
        FILES
            test_lib.h
)

if(MSVC)
    target_compile_options(test_lib PRIVATE /W3 /WX-) # W3 = normal, WX- = No error
else()
    target_compile_options(test_lib PRIVATE -w) # -w disables all warnings (extreme)
endif()