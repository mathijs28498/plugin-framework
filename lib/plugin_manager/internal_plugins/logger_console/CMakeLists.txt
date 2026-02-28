add_library(logger_console STATIC)

target_sources(logger_console
    PRIVATE
        logger_console.c
        logger_console_register.c

    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(logger_console
    PRIVATE
        plugin_manager_include
)

target_compile_definitions(logger_console PRIVATE PLUGIN_INTERFACE_NAME=logger)
