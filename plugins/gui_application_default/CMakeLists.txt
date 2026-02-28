# TODO: Make every plugin work properly like this
#       Maybe start with window plugin?
# TODO: Add option for turning static vs dynamic building, add this to CMakePresets.json
add_library(gui_application_default)

target_sources(gui_application_default
    PRIVATE
        gui_application_default_register.c
        gui_application_default.c
    
    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries(gui_application_default
    PRIVATE
        plugin_manager_include
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(gui_application_default PRIVATE PLUGIN_BUILD_SHARED)
endif()
target_compile_definitions(gui_application_default PRIVATE PLUGIN_INTERFACE_NAME=gui_application)