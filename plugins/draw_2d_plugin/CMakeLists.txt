add_library(draw_2d_plugin SHARED)

target_sources(draw_2d_plugin
    PRIVATE
        draw_2d_plugin.c
        draw_2d_plugin_register.c

    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(draw_2d_plugin
    PRIVATE
        plugin_manager_common
)