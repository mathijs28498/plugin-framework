find_package(Vulkan REQUIRED)

# add_library(renderer_vulkan)
add_library(renderer_vulkan)

target_compile_options(renderer_vulkan PRIVATE ${MY_STRICT_WARNINGS})

target_sources(renderer_vulkan
    PRIVATE
    renderer_vulkan_register.c
    renderer_vulkan_bootstrap.c
    renderer_vulkan_utils.c
    renderer_vulkan_start.c
    renderer_vulkan_render.c
    renderer_vulkan_descriptor_set.c
    renderer_vulkan.c

    PUBLIC
    FILE_SET HEADERS
)

message("TODO: Add other targets")
if(WIN32)
    target_sources(renderer_vulkan PRIVATE renderer_vulkan_platform_win32.c)
endif()

target_link_libraries(renderer_vulkan
    PRIVATE
    Vulkan::Vulkan
    vma
    plugin_sdk
)

include("PluginSDK")
plugin_generate_register_inc(renderer_vulkan
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    TOML_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}"
)

include("RendererShaderGenerator")
target_embed_shader(renderer_vulkan
    GEN_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated"
    SHADER_FILE_PATH "${CMAKE_CURRENT_LIST_DIR}/shaders/gradient.comp"
)
