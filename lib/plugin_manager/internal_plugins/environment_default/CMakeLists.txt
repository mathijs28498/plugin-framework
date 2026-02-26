add_library(environment_default STATIC)

target_sources(environment_default
    PRIVATE
        environment_default.c
        environment_default_register.c

    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(environment_default
    PRIVATE
        plugin_manager_include
)

add_compile_definitions(PLUGIN_INTERFACE_NAME=environment)