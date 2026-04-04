#include "plugin_manager_default.h"
#include <plugin_utils.h>

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

TODO("Remove unnecessary includes")
#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(plugin_manager_default, LOG_LEVEL_DEBUG);
#include <plugin_manager_pm_interface.h>
#include <plugin_sdk_types.h>
#include <environment_interface.h>
#include <environment_pm_interface.h>

#include "plugin_manager_default_register.h"

bool is_lifetime_supported(const PluginMetadata *plugin_metadata, PluginLifetime lifetime)
{
    for (size_t j = 0; j < plugin_metadata->supported_lifetimes_len; j++)
    {
        PluginLifetime supported_lifetime = plugin_metadata->supported_lifetimes[j];
        if (supported_lifetime == lifetime)
        {
            return true;
        }
    }

    return false;
}

int32_t add_plugins_to_scope(const LoggerInterface *logger,
                             const PluginScope *singleton_scope,
                             RegisteredPlugin *registered_plugins,
                             const char **interface_names_to_add, PluginScope *scope)
{
    TODO("Get all dependencies of interfaces")
    int32_t ret;
    for (size_t i = 0; i < GET_ARRAY_LENGTH(interface_names_to_add); i++)
    {
        const char *interface_name_to_add = interface_names_to_add[i];
        ret = add_plugin_to_scope(logger, singleton_scope, registered_plugins, interface_name_to_add, scope);
        if (ret < 0)
        {
            if (logger != NULL)
                LOG_ERR("Unable to add plugin '%s' to scope '%d': %d", interface_name_to_add, scope->lifetime, ret);
            return ret;
        }
    }

    return 0;
}

TODO("Return the added plugins")
int32_t add_plugin_to_scope(const LoggerInterface *logger,
                            const PluginScope *singleton_scope,
                            RegisteredPlugin *registered_plugins,
                            const char *interface_name_to_add, PluginScope *scope)
{
    (void)singleton_scope;
    int32_t ret;

    RegisteredPlugin *registered_plugin_to_add = NULL;
    for (size_t i = 0; i < GET_ARRAY_LENGTH(registered_plugins); i++)
    {
        RegisteredPlugin *registered_plugin = &registered_plugins[i];
        if (strcmp(interface_name_to_add, registered_plugin->metadata->interface_name) == 0)
        {
            registered_plugin_to_add = registered_plugin;
            break;
        }
    }

    if (registered_plugin_to_add == NULL)
    {
        if (logger != NULL)
            LOG_ERR("Unable to add plugin '%s' to scope with lifetime '%d' as it is not registered", interface_name_to_add, scope->lifetime);
        return -1;
    }

    TODO("Maybe check for lifetime promotion, where should this happen? Should this ever happen?")
    if (registered_plugin_to_add->lifetime != PLUGIN_LIFETIME_UNKNOWN &&
        registered_plugin_to_add->lifetime != scope->lifetime)
    {
        if (logger != NULL)
            LOG_ERR("Unable to add plugin '%s' to scope with lifetime '%d' as its scope lifetime '%d' is incompatible",
                    interface_name_to_add, scope->lifetime, registered_plugin_to_add->lifetime);
        return -1;
    }

    if (!is_lifetime_supported(registered_plugin_to_add->metadata, scope->lifetime))
    {
        if (logger != NULL)
            LOG_ERR("Unable to add plugin '%s' to scope with lifetime '%d' the plugin does not support scope lifetime",
                    interface_name_to_add, scope->lifetime);
        return -1;
    }

    const PluginProvider *plugin_provider = registered_plugin_to_add->metadata->provider;

    TODO("Call destroy context when the scope gets destroyed");
    TODO("Figure out what needs to happen to do create_context without malloc, I want the context to live within the scope I think")
    void *context = plugin_provider->create_context();
    TODO("Check if should do != NULL instead")
    if (plugin_provider->init != NULL)
    {
        ret = plugin_provider->init(context);
        if (ret < 0)
        {
            LOG_ERR("Initializing plugin '%s' failed: %d", registered_plugin_to_add->metadata->interface_name, ret);
            return ret;
        }
    }

    TODO("Handle dependencies")
    if (GET_ARRAY_CAPACITY(scope->plugins) == GET_ARRAY_LENGTH(scope->plugins))
    {
        if (logger != NULL)
            LOG_ERR("Tried to add plugin to scope definition when array is full");
        return -1;
    }
    ScopedPlugin *scoped_plugin = &scope->plugins[GET_ARRAY_LENGTH(scope->plugins)];
    scoped_plugin->interface_name = interface_name_to_add;
    scoped_plugin->iface.vtable = plugin_provider->vtable;
    scoped_plugin->iface.context = context;
    GET_ARRAY_LENGTH(scope->plugins) += 1;

    registered_plugin_to_add->lifetime = scope->lifetime;
    if (logger != NULL)
        LOG_DBG("Plugin '%s' added to scope with lifetime '%d'", registered_plugin_to_add->metadata->interface_name, scope->lifetime);

    return 0;
}

int32_t plugin_manager_default_get_singleton(PluginManagerContext *context, const char *interface_name, void **out_iface)
{
    assert(out_iface != NULL);
    return plugin_manager_default_get_scoped(context, &context->singleton_scope, interface_name, out_iface);
}

int32_t plugin_manager_default_get_scoped(struct PluginManagerContext *context, struct PluginScope *scope, const char *interface_name, void **out_iface)
{
    LoggerInterface *logger = context->logger;

    TODO("Check if already in scope, if yes get it, if no create it")
    TODO("If scope is SINGLETON, return err if not available")
    assert(out_iface != NULL);

    for (size_t i = 0; i < GET_ARRAY_LENGTH(scope->plugins); i++)
    {
        ScopedPlugin *plugin = &scope->plugins[i];
        if (strcmp(interface_name, plugin->interface_name) == 0)
        {
            *out_iface = &plugin->iface;
            return 0;
        }
    }
    LOG_ERR("Unable to get plugin '%s' from scope with lifetime '%d'", interface_name, scope->lifetime);
    return -1;
}