add_library(internal_plugins)

target_sources(internal_plugins
    PRIVATE
        environment_plugin.c
        logger_plugin.c

    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(internal_plugins
    PRIVATE
        plugin_manager_common
)
