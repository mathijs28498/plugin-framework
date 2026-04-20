add_library(plugin_manager_default)

target_compile_options(plugin_manager_default PRIVATE ${MY_STRICT_WARNINGS})

target_sources(plugin_manager_default
    PRIVATE
    plugin_manager_default.c
    plugin_manager_default_register.c
    plugin_manager_default_bootstrap.c

    PUBLIC
    FILE_SET HEADERS
)

target_link_libraries(plugin_manager_default
    PRIVATE
    plugin_sdk
    dynamic_metadata_resolver
)

include("PluginSDK")
plugin_generate_register_inc(plugin_manager_default
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}"
)
