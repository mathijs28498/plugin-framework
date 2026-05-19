add_library(draw_default)

target_compile_options(draw_default PRIVATE ${MY_STRICT_WARNINGS})

target_sources(draw_default
    PRIVATE
    draw_default.c
    draw_default_register.c
    draw_default_start.c
    draw_default_render_frame.c

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

target_embed_shader(draw_default
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    SHADER_FILE_PATH "${CMAKE_CURRENT_LIST_DIR}/shaders/colored_triangle.vert"
    SHADER_STAGE "vertex"
)

target_embed_shader(draw_default
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    SHADER_FILE_PATH "${CMAKE_CURRENT_LIST_DIR}/shaders/colored_triangle_mesh.vert"
    SHADER_STAGE "vertex"
)

target_embed_shader(draw_default
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    SHADER_FILE_PATH "${CMAKE_CURRENT_LIST_DIR}/shaders/colored_triangle.frag"
    SHADER_STAGE "fragment"
)