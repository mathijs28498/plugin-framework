add_library(render_graph_default)

target_compile_options(render_graph_default PRIVATE ${MY_STRICT_WARNINGS})

target_sources(render_graph_default
    PRIVATE
    render_graph_default_register.c
    render_graph_default.c

    PUBLIC
    FILE_SET HEADERS
)

target_link_libraries(render_graph_default
    PRIVATE
    plugin_sdk
)

include("pluginSDK")
plugin_generate_register_inc(render_graph_default
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}"
)