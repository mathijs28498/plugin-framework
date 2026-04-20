#include "plugin_manager_default.h"
#include <plugin_sdk/plugin_utils.h>

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

TODO("Remove unnecessary includes")
#include <plugin_sdk/logger/v1/logger_interface.h>
LOGGER_INTERFACE_REGISTER(plugin_manager_default, LOG_LEVEL_WARNING);
#include <plugin_sdk/plugin_manager/v1/plugin_manager_pm_interface.h>
#include <plugin_sdk/plugin_sdk_types/v1/plugin_sdk_types.h>
#include <plugin_sdk/environment/v1/environment_interface.h>
#include <plugin_sdk/environment/v1/environment_pm_interface.h>

#include "plugin_manager_default_register.h"

bool is_lifetime_supported(const PluginMetadata *plugin_metadata, PluginLifetime lifetime)
{
    assert(plugin_metadata != NULL);

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

RegisteredPlugin *get_registered_plugin_by_interface_name(RegisteredPlugin *registered_plugins, const char *interface_name, size_t *out_index)
{
    assert(registered_plugins != NULL);
    assert(interface_name != NULL);

    for (size_t i = 0; i < GET_ARRAY_LENGTH(registered_plugins); i++)
    {
        RegisteredPlugin *registered_plugin = &registered_plugins[i];
        if (strcmp(interface_name, registered_plugin->metadata->interface_name) == 0)
        {
            if (out_index != NULL)
                *out_index = i;
            return registered_plugin;
        }
    }

    return NULL;
}

void resolve_plugin_bitfield_dependencies(RegisteredPlugin *registered_plugins, bool *plugin_bitfield)
{
    assert(registered_plugins != NULL);
    assert(plugin_bitfield != NULL);
    assert(GET_ARRAY_LENGTH(plugin_bitfield) == GET_ARRAY_LENGTH(registered_plugins));

    size_t plugin_bitfield_len = GET_ARRAY_LENGTH(plugin_bitfield);
    for (size_t i = 0; i < plugin_bitfield_len; i++)
    {
        // Loop over the bitfield backwards, as this is already topologically sorted
        size_t current_index = plugin_bitfield_len - i - 1;
        const PluginMetadata *metadata = registered_plugins[current_index].metadata;
        if (!plugin_bitfield[current_index])
        {
            continue;
        }

        for (size_t j = 0; j < metadata->dependencies_len; j++)
        {
            const char *dependency_interface_name = metadata->dependencies[j].interface_name;
            size_t registered_plugin_index;
            (void)get_registered_plugin_by_interface_name(registered_plugins, dependency_interface_name, &registered_plugin_index);
            plugin_bitfield[registered_plugin_index] = true;
        }
    }
}

bool is_plugin_added(PluginScope *singleton_scope, PluginScope *scope, const char *interface_name, ScopedPlugin **out_plugin)
{
    assert(singleton_scope != NULL);
    assert(scope != NULL);
    assert(interface_name != NULL);

    for (size_t i = 0; i < GET_ARRAY_LENGTH(scope->plugins); i++)
    {
        ScopedPlugin *plugin = &scope->plugins[i];
        if (strcmp(plugin->interface_name, interface_name) == 0)
        {
            if (out_plugin != NULL)
                *out_plugin = plugin;
            return true;
        }
    }

    for (size_t i = 0; i < GET_ARRAY_LENGTH(singleton_scope->plugins); i++)
    {
        ScopedPlugin *plugin = &singleton_scope->plugins[i];
        if (strcmp(plugin->interface_name, interface_name) == 0)
        {
            if (out_plugin != NULL)
                *out_plugin = plugin;
            return true;
        }
    }

    return false;
}

// This assumes all the lifetimes and dependencies are correct and present added
int32_t add_plugin_to_scope_unchecked(const LoggerInterface *logger,
                                      PluginScope *singleton_scope,
                                      RegisteredPlugin *plugin,
                                      PluginScope *scope,
                                      ScopedPluginInterface **out_iface)
{
    assert(singleton_scope != NULL);
    assert(plugin != NULL);
    assert(scope != NULL);

    int32_t ret;

    const PluginProvider *plugin_provider = plugin->metadata->provider;

    TODO("Figure out what needs to happen to do create_context without malloc, I want the context to live within the scope I think")
    void *context = plugin_provider->create_context();

    for (size_t i = 0; i < plugin->metadata->dependencies_len; i++)
    {
        const char *dependency_interface_name = plugin->metadata->dependencies[i].interface_name;
        ScopedPlugin *dependency_plugin;
        bool plugin_is_added = is_plugin_added(singleton_scope, scope, dependency_interface_name, &dependency_plugin);

        if (!plugin_is_added)
        {
            if (logger != NULL)
                LOG_ERR(logger, "Dependency '%s' not available in scope", dependency_interface_name);
            return -1;
        }

        TODO("Make this into an array injection where all get set in 1 call");
        plugin_provider->inject_dependency(context, dependency_interface_name, &dependency_plugin->iface);
    }

    if (plugin_provider->init != NULL)
    {
        ret = plugin_provider->init(context);
        if (ret < 0)
        {
            if (logger != NULL)
                LOG_ERR(logger, "Initializing plugin '%s' failed: %d", plugin->metadata->interface_name, ret);
            return ret;
        }
    }

    ScopedPlugin scoped_plugin = {
        .interface_name = plugin->metadata->interface_name,
        .iface.vtable = plugin_provider->vtable,
        .iface.context = context,
    };
    ARRAY_PUSH_CHECKED(scope->plugins, scoped_plugin, {
        if (logger != NULL)
            LOG_ERR(logger, "Tried to add plugin to scope definition when array is full");
        return -1;
    });

    if (out_iface != NULL)
    {
        *out_iface = &scope->plugins[GET_ARRAY_LENGTH(scope->plugins) - 1].iface;
    }

    plugin->lifetime = scope->lifetime;
    if (logger != NULL)
        LOG_DBG(logger, "Plugin '%s' added to scope with lifetime '%d'", plugin->metadata->interface_name, scope->lifetime);

    return 0;
}

int32_t add_plugins_from_bitfield(
    const LoggerInterface *logger,
    PluginScope *singleton_scope,
    RegisteredPlugin *registered_plugins,
    PluginScope *scope,
    bool *plugin_bitfield,
    ScopedPluginInterface **out_plugin_interface)
{
    assert(singleton_scope != NULL);
    assert(registered_plugins != NULL);
    assert(scope != NULL);
    assert(plugin_bitfield != NULL);
    assert(GET_ARRAY_LENGTH(plugin_bitfield) == GET_ARRAY_LENGTH(registered_plugins));

    int32_t ret;
    size_t plugin_bitfield_len = GET_ARRAY_LENGTH(plugin_bitfield);
    const char *out_plugin_name = NULL;
    if (out_plugin_interface != NULL)
    {
        for (size_t i = 0; i < plugin_bitfield_len; i++)
        {
            // Loop over the bitfield backwards, as this is already topologically sorted
            size_t current_index = plugin_bitfield_len - i - 1;
            if (!plugin_bitfield[current_index])
            {
                continue;
            }

            out_plugin_name = registered_plugins[current_index].metadata->interface_name;
            break;
        }
    }

    resolve_plugin_bitfield_dependencies(registered_plugins, plugin_bitfield);

    for (size_t i = 0; i < plugin_bitfield_len; i++)
    {
        if (!plugin_bitfield[i])
        {
            continue;
        }

        RegisteredPlugin *registered_plugin = &registered_plugins[i];
        if (is_plugin_added(singleton_scope, scope, registered_plugin->metadata->interface_name, NULL))
        {
            continue;
        }

        if (registered_plugin->lifetime != PLUGIN_LIFETIME_UNKNOWN && registered_plugin->lifetime != scope->lifetime)
        {
            if (logger != NULL)
                LOG_ERR(logger, "Unable to add plugin '%s' to scope with lifetime '%d' as its scope lifetime '%d' is incompatible",
                        registered_plugin->metadata->interface_name, scope->lifetime, registered_plugin->lifetime);
            return -1;
        }

        if (!is_lifetime_supported(registered_plugin->metadata, scope->lifetime))
        {
            if (logger != NULL)
                LOG_ERR(logger, "Unable to add plugin '%s' to scope with lifetime '%d' the plugin does not support scope lifetime",
                        registered_plugin->metadata->interface_name, scope->lifetime);
            return -1;
        }

        ScopedPluginInterface *added_plugin_interface;
        ret = add_plugin_to_scope_unchecked(logger, singleton_scope, registered_plugin, scope, &added_plugin_interface);
        if (ret < 0)
        {
            LOG_ERR(logger, "Unable to add plugin: %d", ret);
            return -1;
        }

        if (out_plugin_interface != NULL &&
            strcmp(registered_plugin->metadata->interface_name, out_plugin_name) == 0)
        {
            *out_plugin_interface = added_plugin_interface;
        }
    }

    return 0;
}

int32_t add_plugins_to_scope(const LoggerInterface *logger,
                             PluginScope *singleton_scope,
                             RegisteredPlugin *registered_plugins,
                             const char **interface_names_to_add,
                             PluginScope *scope)
{
    assert(logger != NULL);
    assert(singleton_scope != NULL);
    assert(registered_plugins != NULL);
    assert(interface_names_to_add != NULL);
    assert(scope != NULL);

    int ret;
    CREATE_ARRAY_WITH_LEN(bool, plugin_bitfield, MAX_REGISTERED_PLUGINS_LEN, GET_ARRAY_LENGTH(registered_plugins));

    for (size_t i = 0; i < GET_ARRAY_LENGTH(interface_names_to_add); i++)
    {
        size_t registered_plugin_index;
        (void)get_registered_plugin_by_interface_name(registered_plugins, interface_names_to_add[i], &registered_plugin_index);
        plugin_bitfield[registered_plugin_index] = true;
    }

    ret = add_plugins_from_bitfield(logger, singleton_scope, registered_plugins, scope, plugin_bitfield, NULL);
    if (ret < 0)
    {
        LOG_ERR(logger, "Unable to add plugins from bitfield: %d", ret);
        return -1;
    }

    return 0;
}

int32_t add_plugin_to_scope(const LoggerInterface *logger,
                            PluginScope *singleton_scope,
                            RegisteredPlugin *registered_plugins,
                            const char *interface_name_to_add,
                            PluginScope *scope,
                            ScopedPluginInterface **out_iface)
{
    assert(singleton_scope != NULL);
    assert(registered_plugins != NULL);
    assert(interface_name_to_add != NULL);
    assert(scope != NULL);

    int ret;
    CREATE_ARRAY_WITH_LEN(bool, plugin_bitfield, MAX_REGISTERED_PLUGINS_LEN, GET_ARRAY_LENGTH(registered_plugins));

    size_t registered_plugin_index;
    (void)get_registered_plugin_by_interface_name(registered_plugins, interface_name_to_add, &registered_plugin_index);
    plugin_bitfield[registered_plugin_index] = true;

    ret = add_plugins_from_bitfield(logger, singleton_scope, registered_plugins, scope, plugin_bitfield, out_iface);
    if (ret < 0)
    {
        if (logger != NULL)
            LOG_ERR(logger, "Unable to add plugins from bitfield: %d", ret);
        return -1;
    }

    return 0;
}

int32_t plugin_manager_default_get_singleton(PluginManagerContext *context, const char *interface_name, void **out_iface)
{
    assert(context != NULL);
    assert(interface_name != NULL);
    assert(out_iface != NULL);
    TODO("Add check if it is explicit or not, error when getting implicit plugin")
    return plugin_manager_default_get_scoped(context, &context->singleton_scope, interface_name, out_iface);
}

int32_t plugin_manager_default_get_scoped(struct PluginManagerContext *context, struct PluginScope *scope, const char *interface_name, void **out_iface)
{
    assert(context != NULL);
    assert(scope != NULL);
    assert(interface_name != NULL);
    assert(out_iface != NULL);

    LoggerInterface *logger = context->deps.logger;

    TODO("Check if already in scope, if yes get it, if no create it")
    TODO("If scope is SINGLETON, return err if not available")

    for (size_t i = 0; i < GET_ARRAY_LENGTH(scope->plugins); i++)
    {
        ScopedPlugin *plugin = &scope->plugins[i];
        if (strcmp(interface_name, plugin->interface_name) == 0)
        {
            *out_iface = &plugin->iface;
            return 0;
        }
    }
    LOG_ERR(logger, "Unable to get plugin '%s' from scope with lifetime '%d'", interface_name, scope->lifetime);
    return -1;
}

int32_t plugin_manager_default_shutdown_scope(RegisteredPlugin *registered_plugins, PluginScope *scope)
{
    assert(registered_plugins != NULL);
    assert(scope != NULL);

    int32_t ret;

    size_t plugins_len = GET_ARRAY_LENGTH(scope->plugins);
    for (size_t i = 0; i < plugins_len; i++)
    {
        // Loop over the plugins backwards, as this is topologically sorted, to shut them down in descending order
        size_t current_index = plugins_len - i - 1;
        ScopedPlugin *plugin = &scope->plugins[current_index];

        RegisteredPlugin *registered_plugin = get_registered_plugin_by_interface_name(registered_plugins, plugin->interface_name, NULL);
        const PluginProvider *plugin_provider = registered_plugin->metadata->provider;

        if (plugin_provider->shutdown != NULL)
        {

            ret = plugin_provider->shutdown(plugin->iface.context);
            if (ret < 0)
            {
                TODO("Add error log here");
                return -1;
            }
        }

        ret = plugin_provider->destroy_context(plugin->iface.context);
        if (ret < 0)
        {
            TODO("Add error log here");
            return -1;
        }
    }

    return 0;
}