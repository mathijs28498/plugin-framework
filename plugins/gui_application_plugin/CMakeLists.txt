add_library(gui_application_plugin SHARED)

target_sources(gui_application_plugin
    PRIVATE
        gui_application_plugin_register.c
        gui_application_plugin.c
    
    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(gui_application_plugin
    PRIVATE
        plugin_manager_common
)