add_library(plugin_manager_include INTERFACE)

target_sources(plugin_manager_include
    INTERFACE
    FILE_SET HEADERS
)

if(WIN32 AND PLUGIN_MANAGER_GUI)
    target_compile_definitions(plugin_manager_bootloader PRIVATE WINDOWS_GUI=1)
endif()