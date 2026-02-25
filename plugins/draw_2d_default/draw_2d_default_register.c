#include "draw_2d_default_register.h"

#include <stdint.h>

#include <logger_interface.h>
#include <plugin_sdk.h>
#include <draw_2d_interface.h>

#include "draw_2d_default.h"

#define REGISTER_DEPENDENCIES(X) \
    X(LoggerInterface, logger, logger)

PLUGIN_REGISTER_DEPENDENCIES(Draw2dInterfaceContext, REGISTER_DEPENDENCIES);

Draw2dInterface *get_interface()
{
    static Draw2dInterfaceContext context = {0};

    static Draw2dInterface iface = {
        .context = &context,
    };

    return &iface;
}

PLUGIN_REGISTER_INTERFACE(get_interface, Draw2dInterface);