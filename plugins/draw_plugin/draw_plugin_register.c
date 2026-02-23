#include "draw_plugin_register.h"

#include <stdint.h>

#include <logger_interface.h>
#include <plugin_sdk.h>
#include <draw_interface.h>

#include "draw_plugin.h"

#define PLUGIN_INTERFACE_NAME draw

#define REGISTER_DEPENDENCIES(X) \
    X(LoggerInterface, logger, logger)

PLUGIN_REGISTER_DEPENDENCIES(DrawInterfaceContext, REGISTER_DEPENDENCIES);

DrawInterface *get_interface()
{
    static DrawInterfaceContext context = {0};

    static DrawInterface iface = {
        .context = &context,

        .present = draw_plugin_present,
    };

    return &iface;
}

PLUGIN_REGISTER_INTERFACE(get_interface, DrawInterface);