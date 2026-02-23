#include "gui_application_plugin_register.h"

#include <gui_application_interface.h>
#include <plugin_sdk.h>
#include <logger_interface.h>
#include <window_interface.h>
#include <input_interface.h>
#include <draw_interface.h>

#include "gui_application_plugin.h"

#define PLUGIN_INTERFACE_NAME gui_application

#define PLUGIN_DEPENDENCIES(X)           \
    X(LoggerInterface, logger, logger) \
    X(WindowInterface, window, window) \
    X(DrawInterface, draw, draw)       \
    X(InputInterface, input, input)

PLUGIN_REGISTER_DEPENDENCIES(GuiApplicationInterfaceContext, PLUGIN_DEPENDENCIES);

GuiApplicationInterface *get_interface(void)
{
    static GuiApplicationInterfaceContext context = {0};

    static GuiApplicationInterface iface = {
        .context = &context,

        .run = gui_application_plugin_run,
    };

    return &iface;
}

PLUGIN_REGISTER_INTERFACE(get_interface, GuiApplicationInterface);