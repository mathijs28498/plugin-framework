#include <stdint.h>
#include <plugin_sdk/plugin_manager_interface.h>

#include <plugin_sdk/logger_interface.h>
LOGGER_INTERFACE_REGISTER(main, LOG_LEVEL_DEBUG)
#include <plugin_sdk/window_interface.h>
#include <plugin_sdk/gui_application_interface.h>



int32_t plugin_manager_bootloader_main(PluginManagerInterface *plugin_manager)
{
    int ret;

    LoggerInterface *logger;
    ret = PLUGIN_MANAGER_GET_SINGLETON(plugin_manager, "logger", &logger);
    if (ret < 0)
    {
        return ret;
    }

    LOG_WRN(logger, "This works baby!");

    GuiApplicationInterface *gui_application;
    RETURN_IF_ERROR(logger, ret, PLUGIN_MANAGER_GET_SINGLETON(plugin_manager, "gui_application", &gui_application),
                    "Failed to get plugin_manager plugin: %d", ret);

    WindowInterfaceCreateWindowOptions create_window_options = {
        .window_name = "My app",
    };
    RETURN_IF_ERROR(logger, ret, gui_application_setup(gui_application, &create_window_options),
                    "Failed to setup gui application: %d", ret);
    RETURN_IF_ERROR(logger, ret, gui_application_run(gui_application),
                    "Failed to run gui application: %d", ret);
    return 0;
}