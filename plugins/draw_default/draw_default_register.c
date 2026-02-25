#include "draw_default_register.h"

#include <stdint.h>

#include <logger_interface.h>
#include <plugin_sdk.h>
#include <draw_interface.h>

#include "draw_default.h"

#define REGISTER_DEPENDENCIES(X) \
    X(LoggerInterface, logger, logger)

PLUGIN_REGISTER_DEPENDENCIES(DrawInterfaceContext, REGISTER_DEPENDENCIES);

DrawInterface *get_interface()
{
    static DrawInterfaceContext context = {0};

    static DrawInterface iface = {
        .context = &context,

        .present = draw_default_present,
    };

    return &iface;
}

PLUGIN_REGISTER_INTERFACE(get_interface, DrawInterface);