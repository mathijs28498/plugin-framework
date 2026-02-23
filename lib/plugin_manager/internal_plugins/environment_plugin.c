#include "environment_plugin.h"

#include <environment_interface.h>

#include <stdint.h>

#define PLUGIN_INTERFACE_NAME environment

int32_t get_args(EnvironmentInterfaceContext *context, int *argc, char ***argv)
{
    *argc = context->argc;
    *argv = context->argv;
    return 0;
}

int32_t get_platform_context(EnvironmentInterfaceContext *context, void **platform_context)
{
    *platform_context = context->platform_context;
    return 0;
}

int32_t environment_plugin_set_args(EnvironmentInterfaceContext *context, int argc, char **argv, void *platform_context)
{
    context->argc = argc;
    context->argv = argv;
    context->platform_context = platform_context;
    return 0;
}

EnvironmentInterface *environment_get_interface(void)
{
    static EnvironmentInterfaceContext context = {0};

    static EnvironmentInterface iface = {
        .context = &context,
        .get_args = get_args,
        .get_platform_context = get_platform_context,
    };

    return &iface;
}
