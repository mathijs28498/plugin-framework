#include "plugin_manager_default_bootstrap.h"
#include <plugin_utils.h>

#include <stdint.h>
#include <stdbool.h>
TODO("Make sure this is done in a separate file for platform specific shit")
#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <logger_interface.h>
// LOGGER_INTERFACE_REGISTER(plugin_manager_default_bootstrap, LOG_LEVEL_DEBUG)
LOGGER_INTERFACE_REGISTER(plugin_manager_default_bootstrap, LOG_LEVEL_WARNING)
#include <plugin_manager_pm_interface.h>
#include <plugin_sdk_types.h>
#include <environment_pm_interface.h>

#include "plugin_manager_default_register.h"
#include "plugin_manager_default.h"

TODO("Create a hash for interface name for quicker comparisons")

int32_t resolve_requested_plugins(const LoggerInterface *logger,
                                  const RequestedPlugin *requested_plugins,
                                  const PluginRegistry *plugin_registry,
                                  const PluginDefinition **out_plugin_definitions)
{
    assert(requested_plugins != NULL);
    assert(plugin_registry != NULL);
    assert(out_plugin_definitions != NULL);

    for (size_t i = 0; i < GET_ARRAY_LENGTH(requested_plugins); i++)
    {
        const RequestedPlugin *requested_plugin = &requested_plugins[i];
        bool use_default = strlen(requested_plugin->plugin_name) == 0;
        bool definition_found = false;

        for (size_t j = 0; j < plugin_registry->interface_definitions_len; j++)
        {
            const InterfaceDefinition *interface_definition = &plugin_registry->interface_definitions[j];
            if (strcmp(requested_plugin->interface_name, interface_definition->interface_name) != 0)
            {
                continue;
            }

            const char *plugin_name = use_default
                                          ? interface_definition->default_plugin
                                          : requested_plugin->plugin_name;

            for (size_t k = 0; k < interface_definition->plugin_definitions_len; k++)
            {
                const PluginDefinition *plugin_definition = &interface_definition->plugin_definitions[k];
                if (strcmp(plugin_name, plugin_definition->plugin_name) != 0)
                {
                    continue;
                }

                ARRAY_PUSH_CHECKED(out_plugin_definitions, plugin_definition, {
                    if (logger != NULL)
                        LOG_ERR(logger, "Tried to add plugin definition when array is full");
                    return -1;
                });
                if (logger != NULL)
                    LOG_DBG(logger, "Added requested plugin '%s' - '%s'", requested_plugin->interface_name, requested_plugin->plugin_name);
                definition_found = true;
                break;
            }

            if (definition_found)
            {
                break;
            }
        }

        if (!definition_found)
        {
            if (logger != NULL)
                LOG_ERR(logger, "Requested plugin '%s' - '%s' not found in plugin registry",
                        requested_plugin->interface_name, requested_plugin->plugin_name);
            return -1;
        }
    }

    return 0;
}

int32_t resolve_get_metadata_fn_dynamic(const LoggerInterface *logger,
                                        const char *module_path, const char *target_name, PluginGetMetadata_Fn *out_get_metadata_fn)
{
    assert(module_path != NULL);
    assert(target_name != NULL);
    assert(out_get_metadata_fn != NULL);

    HMODULE handle = LoadLibrary(module_path);
    if (!handle)
    {
        if (logger != NULL)
            LOG_ERR(logger, "Dynamic plugin '%s' not found at '%s'", target_name, module_path);
        return -1;
    }

    char function_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
    snprintf(function_name, sizeof(function_name), "%s_get_plugin_metadata", target_name);

    FARPROC proc_address = GetProcAddress(handle, function_name);

    if (!proc_address)
    {
        if (logger != NULL)
            LOG_ERR(logger, "Dynamic plugin symbol '%s' not found for plugin '%s'", target_name, function_name);
        FreeLibrary(handle);
        return -2;
    }

    *out_get_metadata_fn = (PluginGetMetadata_Fn)proc_address;

    return 0;
}

int32_t get_metadata_from_plugin_definition(const LoggerInterface *logger,
                                            const PluginDefinition *plugin_definition,
                                            const PluginMetadata *const *static_plugin_metadatas,
                                            const PluginMetadata **out_plugin_metadata)
{
    assert(plugin_definition != NULL);
    assert(static_plugin_metadatas != NULL);
    assert(out_plugin_metadata != NULL);

    int32_t ret;

    for (size_t j = 0; j < GET_ARRAY_LENGTH(static_plugin_metadatas); j++)
    {
        const PluginMetadata *static_plugin_metadata = static_plugin_metadatas[j];
        TODO("Do I need to do a plugin comparison too?");
        if (strcmp(plugin_definition->interface_name, static_plugin_metadata->interface_name) == 0)
        {
            *out_plugin_metadata = static_plugin_metadata;
            if (logger != NULL)
                LOG_DBG(logger, "Static plugin metadata for plugin '%s' added", plugin_definition->target_name);
            return 0;
        }
    }

    TODO("Save the module and shut it down when not needed anymore")
    PluginGetMetadata_Fn get_metadata_fn = NULL;
    ret = resolve_get_metadata_fn_dynamic(logger, plugin_definition->module_path, plugin_definition->target_name, &get_metadata_fn);
    if (ret < 0)
    {
        if (logger != NULL)
            LOG_ERR(logger, "Unable to resolve metadata for dynamic plugin '%s': %d", plugin_definition->target_name, ret);
        return ret;
    }

    *out_plugin_metadata = get_metadata_fn();
    if (*out_plugin_metadata == NULL)
    {
        if (logger != NULL)
            LOG_ERR(logger, "Dynamic plugin metadata for '%s' is NULL", plugin_definition->target_name);
        return -1;
    }

    if (logger != NULL)
        LOG_DBG(logger, "Dynamic plugin metadata for plugin '%s' added", plugin_definition->target_name);
    return 0;
}

int32_t resolve_plugin_metadatas(const LoggerInterface *logger,
                                 const PluginDefinition **plugin_definitions,
                                 const PluginMetadata *const *static_plugin_metadatas,
                                 RegisteredPlugin *out_registered_plugins)
{
    assert(plugin_definitions != NULL);
    assert(static_plugin_metadatas != NULL);
    assert(out_registered_plugins != NULL);

    int32_t ret;
    for (size_t i = 0; i < GET_ARRAY_LENGTH(plugin_definitions); i++)
    {
        const PluginDefinition *plugin_definition = plugin_definitions[i];

        const PluginMetadata *registered_plugin_metadata = NULL;
        ret = get_metadata_from_plugin_definition(logger, plugin_definition, static_plugin_metadatas, &registered_plugin_metadata);
        if (ret < 0)
        {
            if (logger != NULL)
                LOG_ERR(logger, "Unable to get metadata from plugin definition for '%s'", plugin_definition->target_name);
            return ret;
        }

        RegisteredPlugin new_registered_plugin = {
            .lifetime = PLUGIN_LIFETIME_UNKNOWN,
            .metadata = registered_plugin_metadata,
        };

        ARRAY_PUSH_CHECKED(out_registered_plugins, new_registered_plugin, {
            if (logger != NULL)
                LOG_ERR(logger, "Tried to add registered plugin when array is full");
            return -1;
        });

        if (logger != NULL)
            LOG_DBG(logger, "Added registered plugin '%s'", plugin_definition->target_name);
    }

    return 0;
}

int32_t resolve_plugin_metadata_dependencies(const LoggerInterface *logger,
                                             const RegisteredPlugin *registered_plugins, size_t resolved_plugin_metadata_offset,
                                             RequestedPlugin *out_requested_plugins)
{
    assert(registered_plugins != NULL);
    assert(out_requested_plugins != NULL);

    for (size_t i = resolved_plugin_metadata_offset; i < GET_ARRAY_LENGTH(registered_plugins); i++)
    {
        const PluginMetadata *plugin_metadata = registered_plugins[i].metadata;
        for (size_t j = 0; j < plugin_metadata->dependencies_len; j++)
        {
            const PluginDependency *plugin_dependency = &plugin_metadata->dependencies[j];

            bool dependency_already_present = false;

            for (size_t k = 0; k < GET_ARRAY_LENGTH(registered_plugins); k++)
            {
                const PluginMetadata *plugin_metadata_to_check = registered_plugins[k].metadata;
                if (strcmp(plugin_dependency->interface_name, plugin_metadata_to_check->interface_name) == 0)
                {
                    dependency_already_present = true;
                    break;
                }
            }

            if (dependency_already_present)
            {
                if (logger != NULL)
                    LOG_DBG(logger, "Don't request dependency '%s' as it has already been added", plugin_dependency->interface_name);
                continue;
            }

            for (size_t k = 0; k < GET_ARRAY_LENGTH(out_requested_plugins); k++)
            {
                const RequestedPlugin *requested_plugin_to_check = &out_requested_plugins[k];
                if (strcmp(plugin_dependency->interface_name, requested_plugin_to_check->interface_name) == 0)
                {
                    dependency_already_present = true;
                    break;
                }
            }

            if (dependency_already_present)
            {
                if (logger != NULL)
                    LOG_DBG(logger, "Don't request dependency '%s' as it has already been requested", plugin_dependency->interface_name);
                continue;
            }

            // The dependency is not plugin metadatas, and is not already requested, now we will request it
            RequestedPlugin new_requested_plugin = {
                .interface_name = plugin_dependency->interface_name,
                .plugin_name = "",
                .lifetime = PLUGIN_LIFETIME_UNKNOWN,
            };

            ARRAY_PUSH_CHECKED(out_requested_plugins, new_requested_plugin, {
                if (logger != NULL)
                    LOG_ERR(logger, "Tried to add requested plugin when array is full");
                return -1;
            });

            if (logger != NULL)
                LOG_DBG(logger, "Requesting dependency '%s'", plugin_dependency->interface_name);
        }
    }
    return 0;
}
int32_t load_requested_plugins(const LoggerInterface *logger,
                               const PluginRegistry *plugin_registry,
                               const PluginMetadata *const *static_plugin_metadatas,
                               RequestedPlugin *requested_plugins,
                               RegisteredPlugin *out_registered_plugins)
{
    ;
    assert(plugin_registry != NULL);
    assert(static_plugin_metadatas != NULL);
    assert(requested_plugins != NULL);
    assert(out_registered_plugins != NULL);

    int ret;
    size_t resolved_plugin_metadata_offset = GET_ARRAY_LENGTH(out_registered_plugins);

    SAFE_WHILE(
        GET_ARRAY_LENGTH(requested_plugins) > 0,
        PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH,
        {
            if (logger != NULL)
                LOG_ERR(logger, "Hit maximum dependency solver depth: %d", PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH);
            return -1;
        })
    {
        CREATE_ARRAY(const PluginDefinition *, plugin_definitions, MAX_REGISTERED_PLUGINS_LEN);

        TODO("Add error handling messages")
        ret = resolve_requested_plugins(
            logger,
            requested_plugins,
            plugin_registry,
            plugin_definitions);

        ret = resolve_plugin_metadatas(
            logger,
            plugin_definitions,
            static_plugin_metadatas,
            out_registered_plugins);

        GET_ARRAY_LENGTH(requested_plugins) = 0;

        ret = resolve_plugin_metadata_dependencies(
            logger,
            out_registered_plugins, resolved_plugin_metadata_offset,
            requested_plugins);

        resolved_plugin_metadata_offset = GET_ARRAY_LENGTH(out_registered_plugins);
    }

    return 0;
}

bool plugin_is_dependent_on(const PluginMetadata *dependent_plugin_metadata, const char *dependency_interface_name)
{
    assert(dependent_plugin_metadata != NULL);
    assert(dependency_interface_name != NULL);

    for (size_t i = 0; i < dependent_plugin_metadata->dependencies_len; i++)
    {
        if (strcmp(dependent_plugin_metadata->dependencies[i].interface_name, dependency_interface_name) == 0)
        {
            return true;
        }
    }

    return false;
}

int32_t topologically_sort_registered_plugins(
    const LoggerInterface *logger,
    RegisteredPlugin *registered_plugins,
    size_t already_sorted_len)
{
    assert(registered_plugins != NULL);

    size_t registered_plugins_len = GET_ARRAY_LENGTH(registered_plugins);

    if (registered_plugins_len == 0 || already_sorted_len >= registered_plugins_len)
    {
        return 0;
    }

    CREATE_ARRAY(size_t, sorted_indices, MAX_REGISTERED_PLUGINS_LEN);
    CREATE_ARRAY_WITH_LEN(size_t, ndegrees, MAX_REGISTERED_PLUGINS_LEN, registered_plugins_len);

    for (size_t i = 0; i < registered_plugins_len; i++)
    {
        if (i >= already_sorted_len)
        {
            size_t ndegree = registered_plugins[i].metadata->dependencies_len;
            ndegrees[i] = ndegree;
        }

        if (ndegrees[i] == 0)
        {
            sorted_indices[GET_ARRAY_LENGTH(sorted_indices)] = i;
            GET_ARRAY_LENGTH(sorted_indices) += 1;
        }
    }

    size_t tail = 0;
    SAFE_WHILE(tail < GET_ARRAY_LENGTH(sorted_indices), MAX_REGISTERED_PLUGINS_LEN + 1, {
        if (logger != NULL)
            LOG_ERR(logger, "fatal framework error: Kahn topological sort exceeded max iterations (%d)",
                    MAX_REGISTERED_PLUGINS_LEN + 1);
        return -1;
    })
    {
        const RegisteredPlugin *dependency_plugin = &registered_plugins[sorted_indices[tail]];
        tail++;

        for (size_t i = 0; i < registered_plugins_len; i++)
        {
            if (
                ndegrees[i] == 0 ||
                !plugin_is_dependent_on(registered_plugins[i].metadata, dependency_plugin->metadata->interface_name))
            {
                continue;
            }

            ndegrees[i]--;
            if (ndegrees[i] == 0)
            {
                sorted_indices[GET_ARRAY_LENGTH(sorted_indices)] = i;
                GET_ARRAY_LENGTH(sorted_indices) += 1;
            }
        }
    }

    if (GET_ARRAY_LENGTH(sorted_indices) != registered_plugins_len)
    {
        if (logger != NULL)
        {
            LOG_ERR(logger, "cyclic or missing dependency detected: sorted %zu out of %zu plugins",
                    (size_t)GET_ARRAY_LENGTH(sorted_indices), registered_plugins_len);

            for (size_t i = 0; i < registered_plugins_len; i++)
            {
                if (ndegrees[i] > 0)
                {
                    LOG_ERR(logger, "plugin '%s' (interface: '%s') could not be resolved. It is either missing a dependency or stuck in a cycle.",
                            registered_plugins[i].metadata->plugin_name,
                            registered_plugins[i].metadata->interface_name);
                }
            }
        }
        return -1;
    }

    CREATE_ARRAY_WITH_LEN(RegisteredPlugin, registered_plugins_copy, MAX_REGISTERED_PLUGINS_LEN, registered_plugins_len);
    memcpy(registered_plugins_copy, registered_plugins, sizeof(registered_plugins[0]) * registered_plugins_len);
    for (size_t i = already_sorted_len; i < registered_plugins_len; i++)
    {
        registered_plugins[i] = registered_plugins_copy[sorted_indices[i]];
    }

    return 0;
}

int32_t initialize_plugin_manager_dependencies(
    PluginManagerContext *context,
    const PluginMetadata *plugin_manager_metadata,
    int argc, char **argv, void *platform_context,
    const PluginRegistry *plugin_registry,
    const PluginMetadata *const *static_plugin_metadatas)
{
    assert(context != NULL);
    assert(plugin_manager_metadata != NULL);
    assert(plugin_registry != NULL);
    assert(static_plugin_metadatas != NULL);

    int32_t ret;
    CREATE_ARRAY(RequestedPlugin, requested_plugins, MAX_REGISTERED_PLUGINS_LEN);

    // We "abuse" the resolve dependencies acting like plugin manager registered plugin is a plugin array with length 1
    // so that we can load in the dependencies without loading in plugin_manager itself
    CREATE_INITIALIZED_ARRAY(RegisteredPlugin, plugin_manager_registered_plugin_list,
                             {{
                                 .lifetime = PLUGIN_LIFETIME_SINGLETON,
                                 .metadata = plugin_manager_metadata,
                             }});
    resolve_plugin_metadata_dependencies(NULL, plugin_manager_registered_plugin_list, 0, requested_plugins);

    // Save the dependencies the plugin manager requires to initialize them before step 2
    CREATE_ARRAY_WITH_LEN(const char *, plugin_manager_dependency_interfaces,
                          MAX_REGISTERED_PLUGINS_LEN, GET_ARRAY_LENGTH(requested_plugins));
    for (size_t i = 0; i < GET_ARRAY_LENGTH(plugin_manager_dependency_interfaces); i++)
    {
        plugin_manager_dependency_interfaces[i] = requested_plugins[i].interface_name;
    }

    ret = load_requested_plugins(NULL,
                                 plugin_registry,
                                 static_plugin_metadatas,
                                 requested_plugins,
                                 context->registered_plugins);
    if (ret < 0)
    {
        TODO("Add error log here")
        return ret;
    }

    ret = topologically_sort_registered_plugins(NULL, context->registered_plugins, 0);
    if (ret < 0)
    {
        TODO("Add error log here")
        return ret;
    }

    for (size_t i = 0; i < GET_ARRAY_LENGTH(plugin_manager_dependency_interfaces); i++)
    {
        TODO("Get the added plugin here and put it in a list, this list can then be set as the context->interfaces")
        const char *interface_name_to_add = plugin_manager_dependency_interfaces[i];
        ret = add_plugin_to_scope(context->logger,
                                  &context->singleton_scope, context->registered_plugins,
                                  interface_name_to_add, &context->singleton_scope,
                                  &context->interfaces[i]);
        if (ret < 0)
        {
            TODO("Add error log here");
            // if (context->logger != NULL)
            //     LOG_ERR(logger, "Unable to add plugin '%s' to scope '%d': %d", interface_name_to_add, scope->lifetime, ret);
            return ret;
        }
    }

    environment_pm_set_args(context->environment, argc, argv, platform_context);
    return 0;
}

int32_t seed_explicitly_requested_plugins(
    PluginManagerContext *context,
    const RequestedPlugin *requested_plugins_explicit,
    RequestedPlugin *requested_plugins)
{
    assert(context != NULL);
    assert(requested_plugins_explicit != NULL);
    assert(requested_plugins != NULL);

    LoggerInterface *logger = context->logger;
    for (size_t i = 0; i < GET_ARRAY_LENGTH(requested_plugins_explicit); i++)
    {
        const RequestedPlugin *requested_plugin_explicit = &requested_plugins_explicit[i];
        bool plugin_already_loaded = false;
        // Check if explicitly requested plugins are already loaded as a dependency of the plugin manager
        // If so, do not add them to requested plugins
        for (size_t j = 0; j < GET_ARRAY_LENGTH(context->registered_plugins); j++)
        {
            const PluginMetadata *plugin_metadata = context->registered_plugins[j].metadata;
            if (strcmp(requested_plugin_explicit->interface_name, plugin_metadata->interface_name) == 0)
            {
                plugin_already_loaded = true;
                break;
            }
        }

        if (plugin_already_loaded)
        {
            continue;
        }

        RequestedPlugin requested_plugin = {
            .interface_name = requested_plugin_explicit->interface_name,
            .plugin_name = requested_plugin_explicit->plugin_name,
        };
        ARRAY_PUSH_CHECKED(requested_plugins, requested_plugin, {
            if (logger != NULL)
                LOG_ERR(logger, "Tried to add requested plugin when array is full");
            return -1;
        });
    }

    return 0;
}

int32_t registered_plugin_set_preferred_lifetime(const LoggerInterface *logger, RegisteredPlugin *registered_plugin)
{
    assert(logger != NULL);
    assert(registered_plugin != NULL);

    for (size_t i = 0; i < registered_plugin->metadata->supported_lifetimes_len; i++)
    {
        const PluginLifetime supported_lifetime = registered_plugin->metadata->supported_lifetimes[i];
        if (supported_lifetime == registered_plugin->metadata->preferred_lifetime)
        {
            if (!is_lifetime_supported(registered_plugin->metadata, supported_lifetime))
            {
                LOG_ERR(logger, "plugin '%s' - '%s' does not support preferred lifetime '%d'",
                        registered_plugin->metadata->interface_name, registered_plugin->metadata->plugin_name, supported_lifetime);
                return -1;
            }
            registered_plugin->lifetime = supported_lifetime;
            return 0;
        }
    }

    return 0;
}

int32_t resolve_initial_lifetimes(
    const LoggerInterface *logger,
    const RequestedPlugin *requested_plugins_explicit,
    RegisteredPlugin *registered_plugins)
{
    assert(logger != NULL);
    assert(requested_plugins_explicit != NULL);
    assert(registered_plugins != NULL);

    int32_t ret;

    for (size_t i = 0; i < GET_ARRAY_LENGTH(registered_plugins); i++)
    {
        RegisteredPlugin *registered_plugin = &registered_plugins[i];
        bool lifetime_already_resolved = registered_plugin->lifetime != PLUGIN_LIFETIME_UNKNOWN;

        PluginLifetime requested_lifetime = PLUGIN_LIFETIME_UNKNOWN;

        // First check for explicitly requested lifetimes,
        for (size_t j = 0; j < GET_ARRAY_LENGTH(requested_plugins_explicit); j++)
        {
            const RequestedPlugin *requested_plugin = &requested_plugins_explicit[j];
            if (strcmp(registered_plugin->metadata->interface_name, requested_plugin->interface_name) == 0)
            {
                requested_lifetime = requested_plugin->lifetime;
                break;
            }
        }

        if (requested_lifetime != PLUGIN_LIFETIME_UNKNOWN)
        {
            // The lifetime is explicitly requested
            if (lifetime_already_resolved && registered_plugin->lifetime != requested_lifetime)
            {
                LOG_ERR(logger, "plugin '%s' - '%s' has a resolved lifetime '%d' which is different from the explictly requested lifetime '%d'",
                        registered_plugin->metadata->interface_name, registered_plugin->metadata->plugin_name, registered_plugin->lifetime, requested_lifetime);
                return -1;
            }

            if (!is_lifetime_supported(registered_plugin->metadata, requested_lifetime))
            {
                LOG_ERR(logger, "plugin '%s' - '%s' does not support requested lifetime '%d'",
                        registered_plugin->metadata->interface_name, registered_plugin->metadata->plugin_name, requested_lifetime);
                return -1;
            }

            registered_plugin->lifetime = requested_lifetime;
            continue;
        }

        if (lifetime_already_resolved)
        {
            continue;
        }

        // If preferred lifetime is singleton, set the lifetime to singleton
        if (registered_plugin->metadata->preferred_lifetime == PLUGIN_LIFETIME_SINGLETON)
        {
            ret = registered_plugin_set_preferred_lifetime(logger, registered_plugin);
            if (ret < 0)
            {
                LOG_ERR(logger, "error setting preferred lifetime of plugin '%s' - '%s': %d",
                        registered_plugin->metadata->interface_name, registered_plugin->metadata->plugin_name, ret);
                return ret;
            }
            continue;
        }

        // Set the lifetime to the only one available, if there is only 1 supported
        if (registered_plugin->metadata->supported_lifetimes_len == 1)
        {
            registered_plugin->lifetime = registered_plugin->metadata->supported_lifetimes[0];
            continue;
        }
    }
    return 0;
}

int32_t plugin_manager_default_bootstrap(
    PluginManagerContext *context,
    const PluginMetadata *plugin_manager_metadata,
    int argc, char **argv, void *platform_context,
    const PluginRegistry *plugin_registry,
    const PluginMetadata *const *static_plugin_metadatas,
    const RequestedPlugin *requested_plugins_explicit)
{
    assert(context != NULL);
    assert(plugin_manager_metadata != NULL);
    assert(plugin_registry != NULL);
    assert(static_plugin_metadatas != NULL);
    assert(requested_plugins_explicit != NULL);

    int32_t ret;

    ret = initialize_plugin_manager_dependencies(
        context, plugin_manager_metadata,
        argc, argv, platform_context,
        plugin_registry, static_plugin_metadatas);
    if (ret < 0)
    {
        TODO("Add logging here");
        return ret;
    }

    size_t initial_registered_plugins_len = GET_ARRAY_LENGTH(context->registered_plugins);

    const LoggerInterface *logger = context->logger;

    CREATE_ARRAY(RequestedPlugin, requested_plugins, MAX_REGISTERED_PLUGINS_LEN);
    ret = seed_explicitly_requested_plugins(context, requested_plugins_explicit, requested_plugins);
    if (ret < 0)
    {
        LOG_ERR(logger, "Unable to seed requested plugins: %d", ret);
        return ret;
    }

    ret = load_requested_plugins(
        logger,
        plugin_registry,
        static_plugin_metadatas,
        requested_plugins,
        context->registered_plugins);
    if (ret < 0)
    {
        TODO("Add error log here");
        return ret;
    }

    ret = topologically_sort_registered_plugins(
        logger,
        context->registered_plugins,
        initial_registered_plugins_len);
    if (ret < 0)
    {
        TODO("Add error log here");
        return ret;
    }

    ret = resolve_initial_lifetimes(
        logger,
        requested_plugins_explicit,
        context->registered_plugins);
    if (ret < 0)
    {
        TODO("Add error log here");
        return ret;
    }

    size_t registered_plugin_len = GET_ARRAY_LENGTH(context->registered_plugins);
    CREATE_ARRAY(const char *, singleton_interfaces_to_add, MAX_REGISTERED_PLUGINS_LEN);

    for (size_t i = 0; i < registered_plugin_len; i++)
    {
        TODO("Check if its better to do it backwards or it doesnt matter")
        // const RegisteredPlugin *registered_plugin = &context->registered_plugins[registered_plugin_len - i - 1];
        const RegisteredPlugin *registered_plugin = &context->registered_plugins[i];
        if (registered_plugin->lifetime == PLUGIN_LIFETIME_SINGLETON)
        {
            ARRAY_PUSH_CHECKED(singleton_interfaces_to_add, registered_plugin->metadata->interface_name, {
                LOG_ERR(logger, "Tried to add plugin interface when array is full");
                return -1;
            });
        }
    }

    add_plugins_to_scope(logger, &context->singleton_scope, context->registered_plugins, singleton_interfaces_to_add, &context->singleton_scope);
    return 0;
}