add_library(draw_plugin SHARED)

target_sources(draw_plugin
    PRIVATE
        draw_plugin.c
        draw_plugin_register.c

    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(draw_plugin
    PRIVATE
        plugin_manager_common
)