#include "draw_2d_plugin_register.h"

#include <stdint.h>

#include <logger_interface.h>
#include <plugin_manager_impl.h>
#include <draw_2d_interface.h>

#include "draw_2d_plugin.h"

#define PLUGIN_INTERFACE_NAME draw_2d_interface

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