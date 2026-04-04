#include "plugin_manager_default_register.h"

#include <plugin_manager_interface.h>
#include <plugin_manager_pm_interface.h>
#include <plugin_sdk_types.h>

#include "plugin_manager_default.h"
#include "plugin_manager_default_bootstrap.h"

static const PluginManagerPMVtable plugin_vtable = {
    .public_vtable = {
        .get_singleton = plugin_manager_default_get_singleton,
        .get_scoped = plugin_manager_default_get_scoped,
    },
    .bootstrap = plugin_manager_default_bootstrap,
};

int32_t plugin_init(PluginManagerContext *context)
{
    TODO("Make this into a macro to set capacity");
    GET_ARRAY_CAPACITY(context->registered_plugins) = ARRAY_SIZE(context->registered_plugins);

    context->singleton_scope.lifetime = PLUGIN_LIFETIME_SINGLETON;
    GET_ARRAY_CAPACITY(context->singleton_scope.plugins) = ARRAY_SIZE(context->singleton_scope.plugins);
    return 0;
}

int32_t plugin_shutdown(PluginManagerContext *context)
{
    TODO("Shutdown all plugins, including singletons and scoped ones, in ascending order of topological sort")
    (void) context;
    return 0;
}

#include "plugin_register.c.inc"