add_library(plugin_sdk INTERFACE)

set(INTERFACE_DIR_PATH "include/plugin_sdk")

target_sources(plugin_sdk
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS "include"
    FILES
    "${INTERFACE_DIR_PATH}/draw_2d_interface.h"
    "${INTERFACE_DIR_PATH}/draw_interface.h"
    "${INTERFACE_DIR_PATH}/environment_interface.h"
    "${INTERFACE_DIR_PATH}/environment_pm_interface.h"
    "${INTERFACE_DIR_PATH}/gui_application_interface.h"
    "${INTERFACE_DIR_PATH}/input_interface.h"
    "${INTERFACE_DIR_PATH}/logger_interface.h"
    "${INTERFACE_DIR_PATH}/plugin_framework.h"
    "${INTERFACE_DIR_PATH}/plugin_manager_interface.h"
    "${INTERFACE_DIR_PATH}/plugin_manager_pm_interface.h"
    "${INTERFACE_DIR_PATH}/plugin_sdk_types.h"
    "${INTERFACE_DIR_PATH}/plugin_utils.h"
    "${INTERFACE_DIR_PATH}/renderer_interface.h"
    "${INTERFACE_DIR_PATH}/window_interface.h"
    "${INTERFACE_DIR_PATH}/window_interface_window_event.h"
    "${INTERFACE_DIR_PATH}/window_interface_window_event_enums.h"
)

if(WIN32 AND PLUGIN_MANAGER_GUI)
    target_compile_definitions(plugin_sdk INTERFACE WINDOWS_GUI=1)
endif()

if(BUILD_SHARED_LIBS)
    target_compile_definitions(plugin_sdk INTERFACE PLUGIN_BUILD_SHARED)
endif()