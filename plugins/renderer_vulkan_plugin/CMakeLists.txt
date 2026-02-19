add_library(renderer_vulkan_plugin SHARED)

target_sources(renderer_vulkan_plugin
    PRIVATE
        renderer_vulkan_plugin.c
        renderer_vulkan_plugin_register.c

    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(renderer_vulkan_plugin
    PRIVATE
        plugin_manager_common
)