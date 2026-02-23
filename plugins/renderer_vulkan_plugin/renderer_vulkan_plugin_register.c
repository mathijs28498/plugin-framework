#include "renderer_vulkan_plugin_register.h"

#include <stdint.h>

#include <logger_interface.h>
#include <plugin_sdk.h>
#include <renderer_interface.h>

#include "renderer_vulkan_plugin.h"

#define PLUGIN_INTERFACE_NAME renderer

#define REGISTER_DEPENDENCIES(X) \
    X(LoggerInterface, logger, logger)

PLUGIN_REGISTER_DEPENDENCIES(RendererInterfaceContext, REGISTER_DEPENDENCIES);

RendererInterface *get_interface()
{
    static RendererInterfaceContext context = {0};

    static RendererInterface iface = {
        .context = &context,
    };

    return &iface;
}

PLUGIN_REGISTER_INTERFACE(get_interface, RendererInterface);