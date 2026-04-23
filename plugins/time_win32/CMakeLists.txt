add_library(time_win32)

target_compile_options(time_win32 PRIVATE ${MY_STRICT_WARNINGS})

target_sources(time_win32
    PRIVATE
    time_win32_register.c
    time_win32.c

    PUBLIC
    FILE_SET HEADERS
)

target_link_libraries(time_win32
    PRIVATE
    plugin_sdk
)

include("pluginSDK")
plugin_generate_register_inc(time_win32
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}"
)