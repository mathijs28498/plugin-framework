add_library(cglm INTERFACE)

target_include_directories(cglm
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# target_sources(cglm
# INTERFACE
# FILE_SET HEADERS
# BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
# FILES
# ${CMAKE_CURRENT_SOURCE_DIR}/cglm/types.h
# ${CMAKE_CURRENT_SOURCE_DIR}/cglm/cglm.h

# )
