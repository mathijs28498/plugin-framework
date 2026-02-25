#include "gui_application_default_register.h"

#include <gui_application_interface.h>
#include <plugin_sdk.h>
#include <logger_interface.h>
#include <window_interface.h>
#include <input_interface.h>
#include <draw_interface.h>

#include "gui_application_default.h"

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

        .setup = gui_application_default_setup,
        .run = gui_application_default_run,
    };

    return &iface;
}

PLUGIN_REGISTER_INTERFACE(get_interface, GuiApplicationInterface);