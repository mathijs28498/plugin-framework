# TODO: Add platform independent plugin loader

# TODO: Change name to PluginManager
add_library(plugin_manager)

target_sources(plugin_manager
    PRIVATE
        file_io.c
        plugin_manager_loader.c
        plugin_manager.c
        plugin_registry.c

    # PUBLIC
    #     FILE_SET HEADERS
)

target_link_libraries(plugin_manager
    PRIVATE
        plugin_manager_common
        cjson
        internal_plugins
)

add_subdirectory(internal_plugins)