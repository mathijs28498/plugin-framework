add_library(plugin_sdk INTERFACE)

target_sources(plugin_sdk
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS "include"
    FILES
    "include/plugin_sdk/draw_2d_interface.h"
    "include/plugin_sdk/draw_interface.h"
    "include/plugin_sdk/environment_interface.h"
    "include/plugin_sdk/environment_pm_interface.h"
    "include/plugin_sdk/gui_application_interface.h"
    "include/plugin_sdk/input_interface.h"
    "include/plugin_sdk/logger_interface.h"
    "include/plugin_sdk/plugin_framework.h"
    "include/plugin_sdk/plugin_manager_interface.h"
    "include/plugin_sdk/plugin_manager_pm_interface.h"
    "include/plugin_sdk/plugin_sdk_types.h"
    "include/plugin_sdk/plugin_utils.h"
    "include/plugin_sdk/renderer_interface.h"
    "include/plugin_sdk/window_interface.h"
    "include/plugin_sdk/window_interface_window_event.h"
    "include/plugin_sdk/window_interface_window_event_enums.h"
)

if(WIN32 AND PLUGIN_MANAGER_GUI)
    target_compile_definitions(plugin_sdk INTERFACE WINDOWS_GUI=1)
endif()
