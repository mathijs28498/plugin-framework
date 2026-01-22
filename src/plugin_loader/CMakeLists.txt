# TODO: Add platform independent plugin loader

add_library(plugin_loader)

target_sources(plugin_loader
    PRIVATE
        plugin_loader.c

    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(plugin_loader
    plugin_common
)