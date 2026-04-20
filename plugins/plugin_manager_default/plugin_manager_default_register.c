#include "plugin_manager_default_register.h"

#include <plugin_sdk/plugin_manager_interface.h>
#include <plugin_sdk/plugin_manager_pm_interface.h>
#include <plugin_sdk/plugin_sdk_types.h>

#include "plugin_manager_default.h"
#include "plugin_manager_default_bootstrap.h"

static const PluginManagerPMVtable plugin_vtable = {
    .public_vtable = {
        .get_singleton = plugin_manager_default_get_singleton,
        .get_scoped = plugin_manager_default_get_scoped,
    },
    .bootstrap = plugin_manager_default_bootstrap,
};

static int32_t plugin_init(PluginManagerContext *context)
{
    TODO("Make this into a macro to set capacity");
    SET_ARRAY_FIELD_CAPACITY(context->registered_plugins);

    context->singleton_scope.lifetime = PLUGIN_LIFETIME_SINGLETON;
    SET_ARRAY_FIELD_CAPACITY(context->singleton_scope.plugins)
    return 0;
}

static int32_t plugin_shutdown(PluginManagerContext *context)
{
    int32_t ret = plugin_manager_default_shutdown_scope(context->registered_plugins, &context->singleton_scope);
    if (ret < 0)
    {
        TODO("Add error log here");
        return ret;
    }
    TODO("Shutdown any scopes that are still open");
    
    return 0;
}

#include "plugin_register.c.inc"