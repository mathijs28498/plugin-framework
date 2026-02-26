#include "environment_default_register.h"

#include <stdint.h>

#include <logger_interface.h>
#include <plugin_sdk.h>
#include <plugin_utils.h>
#include <environment_interface.h>

#include "environment_default.h"

TODO("Make this work with get_interface with naming collision figure out how to")
EnvironmentInterface *env_get_interface(void)
{
    static EnvironmentInterfaceContext context = {0};

    static EnvironmentInterface iface = {
        .context = &context,

        .get_args = environment_default_get_args,
        .get_platform_context = environment_default_get_platform_context,
    };

    return &iface;
}

PLUGIN_REGISTER_INTERFACE(env_get_interface, EnvironmentInterface);