add_library(gui_application_default)

target_compile_options(gui_application_default PRIVATE ${MY_STRICT_WARNINGS})

target_sources(gui_application_default
    PRIVATE
    gui_application_default_register.c
    gui_application_default.c

    PUBLIC
    FILE_SET HEADERS
)

target_link_libraries(gui_application_default
    PRIVATE
    plugin_sdk
)

include("PluginSDK")
plugin_generate_register_inc(gui_application_default
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}"
)