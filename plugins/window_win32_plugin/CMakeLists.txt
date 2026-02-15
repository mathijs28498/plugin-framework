add_library(window_win32_plugin SHARED)

target_sources(window_win32_plugin
    PRIVATE
        window_win32_plugin.c
        window_win32_plugin_register.c

    PUBLIC
        FILE_SET HEADERS
)

target_compile_definitions(window_win32_plugin PRIVATE UNICODE _UNICODE COBJMACROS)

target_link_libraries(window_win32_plugin
    PRIVATE
        plugin_manager_common
)