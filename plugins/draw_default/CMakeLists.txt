add_library(draw_default)

target_compile_options(draw_default PRIVATE ${MY_STRICT_WARNINGS})

target_sources(draw_default
    PRIVATE
    draw_default.c
    draw_default_register.c

    PUBLIC
    FILE_SET HEADERS
)

target_link_libraries(draw_default
    PRIVATE
    plugin_sdk
    cglm
)

include("PluginSDK")
message(STATUS "Generate inc: draw default")
plugin_generate_register_inc(draw_default
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}"
)


include("RendererShaderGenerator")
target_embed_shader(draw_default
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    SHADER_FILE_PATH "${CMAKE_CURRENT_LIST_DIR}/shaders/background.comp"
    SHADER_STAGE "compute"
)