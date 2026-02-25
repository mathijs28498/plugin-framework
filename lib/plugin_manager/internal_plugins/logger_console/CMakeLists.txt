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

add_compile_definitions(PLUGIN_INTERFACE_NAME=logger)
