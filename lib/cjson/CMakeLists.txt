add_library(cjson cJSON.c)

target_sources(cjson
    PUBLIC
        FILE_SET HEADERS
        FILES 
            cJSON.h
)

if(NOT WIN32)
    target_link_libraries(cjson PRIVATE m)
endif()