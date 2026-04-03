#include "plugin_manager_default_bootstrap.h"
#include <plugin_utils.h>

#include <stdint.h>
TODO("Make sure this is done in a separate file for platform specific shit")
#include <Windows.h>
#include <assert.h>
#include <stdio.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(plugin_manager_default_bootstrap, LOG_LEVEL_DEBUG)
#include <plugin_manager_pm_interface.h>
#include <plugin_sdk_types.h>
#include <environment_pm_interface.h>

#include "plugin_manager_default_register.h"
#include "plugin_manager_default.h"

int32_t resolve_requested_plugins(const LoggerInterface *logger,
                                  const RequestedPlugin *requested_plugins, size_t requested_plugins_len,
                                  const PluginRegistry *plugin_registry,
                                  const PluginDefinition **out_plugin_definitions, size_t *out_plugin_definitions_len)
{
    for (size_t i = 0; i < requested_plugins_len; i++)
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

                out_plugin_definitions[*out_plugin_definitions_len] = plugin_definition;
                (*out_plugin_definitions_len)++;
                TODO("Add max size check here")
                if (logger != NULL)
                    LOG_DBG("Added requested plugin '%s' - '%s'", requested_plugin->interface_name, requested_plugin->plugin_name);
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
                LOG_ERR("Requested plugin '%s' - '%s' not found in plugin registry",
                        requested_plugin->interface_name, requested_plugin->plugin_name);
            return -1;
        }
    }

    return 0;
}

int32_t resolve_get_metadata_fn_dynamic(const LoggerInterface *logger,
                                        const char *module_path, const char *target_name, PluginGetMetadata_Fn *out_get_metadata_fn)
{
    assert(out_get_metadata_fn != NULL);

    TODO("Figure out how to destroy the handle if that is necessary, also allow this code to be optionally compiled based on platform/PLUGIN_BUILD_SHARED")
    HMODULE handle = LoadLibrary(module_path);
    if (!handle)
    {
        TODO("Add error log here")
        if (logger != NULL)
            LOG_ERR("Dynamic plugin '%s' not found at '%s'", target_name, module_path);
        return -1;
    }

    char function_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
    snprintf(function_name, sizeof(function_name), "%s_get_plugin_metadata", target_name);

    FARPROC proc_address = GetProcAddress(handle, function_name);

    if (!proc_address)
    {
        // TODO: Add error log here (Symbol not found)
        TODO("Add error log here")
        if (logger != NULL)
            LOG_ERR("Dynamic plugin symbol '%s' not found for plugin '%s'", target_name, function_name);
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
    int32_t ret;

    for (size_t j = 0; j < GET_ARRAY_LENGTH(static_plugin_metadatas); j++)
    {
        const PluginMetadata *static_plugin_metadata = static_plugin_metadatas[j];
        TODO("Do I need to do a plugin comparison too?");
        if (strcmp(plugin_definition->interface_name, static_plugin_metadata->interface_name) == 0)
        {
            *out_plugin_metadata = static_plugin_metadata;
            if (logger != NULL)
                LOG_DBG("Static plugin metadata for plugin '%s' added", plugin_definition->target_name);
            return 0;
        }
    }

    PluginGetMetadata_Fn get_metadata_fn = NULL;
    ret = resolve_get_metadata_fn_dynamic(logger, plugin_definition->module_path, plugin_definition->target_name, &get_metadata_fn);
    if (ret < 0)
    {
        if (logger != NULL)
            LOG_ERR("Unable to resolve metadata for dynamic plugin '%s': %d", plugin_definition->target_name, ret);
        return ret;
    }

    *out_plugin_metadata = get_metadata_fn();
    if (*out_plugin_metadata == NULL)
    {
        if (logger != NULL)
            LOG_ERR("Dynamic plugin metadata for '%s' is NULL", plugin_definition->target_name);
        return -1;
    }

    if (logger != NULL)
        LOG_DBG("Dynamic plugin metadata for plugin '%s' added", plugin_definition->target_name);
    return 0;
}

int32_t resolve_plugin_metadatas(const LoggerInterface *logger,
                                 const PluginDefinition **plugin_definitions, size_t plugin_definitions_len,
                                 const PluginMetadata *const *static_plugin_metadatas,
                                 RegisteredPlugin *out_registered_plugins, size_t *out_registered_plugins_len)
{
    int32_t ret;
    for (size_t i = 0; i < plugin_definitions_len; i++)
    {
        const PluginDefinition *plugin_definition = plugin_definitions[i];

        const PluginMetadata *registered_plugin_metadata = NULL;
        ret = get_metadata_from_plugin_definition(logger, plugin_definition, static_plugin_metadatas, &registered_plugin_metadata);
        if (ret < 0)
        {
            if (logger != NULL)
                LOG_ERR("Unable to get metadata from plugin definition for '%s'", plugin_definition->target_name);
            return ret;
        }

        TODO("Add max size check here")
        RegisteredPlugin *new_registered_plugin = &out_registered_plugins[*out_registered_plugins_len];
        new_registered_plugin->lifetime = PLUGIN_LIFETIME_UNKNOWN;
        new_registered_plugin->metadata = registered_plugin_metadata;
        (*out_registered_plugins_len)++;
        if (logger != NULL)
            LOG_DBG("Added registered plugin '%s'", plugin_definition->target_name);
    }

    return 0;
}

int32_t resolve_plugin_metadata_dependencies(const LoggerInterface *logger,
                                             const RegisteredPlugin *registered_plugins, size_t registered_plugins_len, size_t resolved_plugin_metadata_offset,
                                             RequestedPlugin *out_requested_plugins, size_t *out_requested_plugins_len)
{
    for (size_t i = resolved_plugin_metadata_offset; i < registered_plugins_len; i++)
    {
        const PluginMetadata *plugin_metadata = registered_plugins[i].metadata;
        for (size_t j = 0; j < plugin_metadata->dependencies_len; j++)
        {
            const PluginDependency *plugin_dependency = &plugin_metadata->dependencies[j];

            bool dependency_already_present = false;

            for (size_t k = 0; k < registered_plugins_len; k++)
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
                    LOG_DBG("Don't request dependency '%s' as it has already been added", plugin_dependency->interface_name);
                continue;
            }

            for (size_t k = 0; k < *out_requested_plugins_len; k++)
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
                    LOG_DBG("Don't request dependency '%s' as it has already been requested", plugin_dependency->interface_name);
                continue;
            }

            // The dependency is not plugin metadatas, and is not already requested, now we will request it
            out_requested_plugins[*out_requested_plugins_len].interface_name = plugin_dependency->interface_name;
            out_requested_plugins[*out_requested_plugins_len].plugin_name = "";
            (*out_requested_plugins_len)++;
            if (logger != NULL)
                LOG_DBG("Requesting dependency '%s'", plugin_dependency->interface_name);
            TODO("Add max size check here")
        }
    }
    return 0;
}
int32_t load_requested_plugins(const LoggerInterface *logger,
                               const PluginRegistry *plugin_registry,
                               const PluginMetadata *const *static_plugin_metadatas,
                               RequestedPlugin *requested_plugins, size_t requested_plugins_len,
                               RegisteredPlugin *out_registered_plugins, size_t *out_registered_plugins_len)
{
    int ret;
    size_t resolved_plugin_metadata_offset = *out_registered_plugins_len;

    SAFE_WHILE(
        requested_plugins_len > 0,
        PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH,
        {
            TODO("Add error log")
            if (logger != NULL)
                LOG_ERR("Hit maximum dependency solver depth: %d", PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH);
            return -1;
        })
    {
        const PluginDefinition *plugin_definitions[MAX_REGISTERED_PLUGINS_LEN];
        size_t plugin_definitions_len = 0;

        TODO("Add error handling messages")
        ret = resolve_requested_plugins(
            logger,
            requested_plugins, requested_plugins_len,
            plugin_registry,
            plugin_definitions, &plugin_definitions_len);

        ret = resolve_plugin_metadatas(
            logger,
            plugin_definitions, plugin_definitions_len,
            static_plugin_metadatas,
            out_registered_plugins, out_registered_plugins_len);

        requested_plugins_len = 0;

        ret = resolve_plugin_metadata_dependencies(
            logger,
            out_registered_plugins, *out_registered_plugins_len, resolved_plugin_metadata_offset,
            requested_plugins, &requested_plugins_len);

        resolved_plugin_metadata_offset = *out_registered_plugins_len;
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
    int32_t ret;

    // We "abuse" the resolve dependencies acting like plugin manager registered plugin is a plugin array with length 1
    // so that we can load in the dependencies without loading in plugin_manager itself
    RegisteredPlugin plugin_manager_registered_plugin = {
        .lifetime = PLUGIN_LIFETIME_SINGLETON,
        .metadata = plugin_manager_metadata,
    };
    RequestedPlugin requested_plugins[MAX_REGISTERED_PLUGINS_LEN] = {0};
    size_t requested_plugins_len = 0;
    resolve_plugin_metadata_dependencies(NULL, &plugin_manager_registered_plugin, 1, 0, requested_plugins, &requested_plugins_len);

    // Save the dependencies the plugin manager requires to initialize them before step 2
    const char *plugin_manager_dependency_interfaces[MAX_REGISTERED_PLUGINS_LEN];
    size_t plugin_manager_dependency_interfaces_len = requested_plugins_len;
    for (int i = 0; i < requested_plugins_len; i++)
    {
        plugin_manager_dependency_interfaces[i] = requested_plugins[i].interface_name;
    }

    ret = load_requested_plugins(NULL,
                                 plugin_registry,
                                 static_plugin_metadatas,
                                 requested_plugins, requested_plugins_len,
                                 context->registered_plugins, &context->registered_plugins_len);

    for (int i = 0; i < plugin_manager_dependency_interfaces_len; i++)
    {
        ret = add_plugin_to_scope(context->logger,
                                  &context->singleton_scope,
                                  context->registered_plugins, context->registered_plugins_len,
                                  plugin_manager_dependency_interfaces[i], &context->singleton_scope);
        if (ret < 0)
        {
            TODO("Add error log here")
            return ret;
        }

        ret = plugin_manager_default_get_singleton(
            context, plugin_manager_dependency_interfaces[i],
            (void **)&context->interfaces[i]);

        if (ret < 0)
        {
            TODO("Add error log here")
            return ret;
        }
    }

    environment_pm_set_args(context->environment, argc, argv, platform_context);
    return 0;
}

int32_t seed_explicitly_requested_plugins(
    PluginManagerContext *context,
    const RequestedPlugin *requested_plugins_explicit,
    RequestedPlugin *requested_plugins, size_t *requested_plugins_len)
{
    for (size_t i = 0; i < GET_ARRAY_LENGTH(requested_plugins_explicit); i++)
    {
        const RequestedPlugin *requested_plugin_explicit = &requested_plugins_explicit[i];
        bool plugin_already_loaded = false;
        // Check if explicitly requested plugins are already loaded as a dependency of the plugin manager
        // If so, do not add them to requested plugins
        for (size_t j = 0; j < context->registered_plugins_len; j++)
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

        requested_plugins[*requested_plugins_len].interface_name = requested_plugin_explicit->interface_name;
        requested_plugins[*requested_plugins_len].plugin_name = requested_plugin_explicit->plugin_name;
        (*requested_plugins_len)++;
    }

    return 0;
}

void registered_plugin_set_preferred_lifetime(RegisteredPlugin *registered_plugin)
{
    for (int i = 0; i < registered_plugin->metadata->supported_lifetimes_len; i++)
    {
        const PluginLifetime supported_lifetime = registered_plugin->metadata->supported_lifetimes[i];
        if (supported_lifetime == registered_plugin->metadata->preferred_lifetime)
        {
            registered_plugin->lifetime = supported_lifetime;
            return;
        }
    }
}

int32_t resolve_registered_plugins_lifetimes(
    const LoggerInterface *logger,
    const RequestedPlugin *requested_plugins_explicit,
    RegisteredPlugin *registered_plugins, size_t registered_plugins_len)
{
    for (int i = 0; i < registered_plugins_len; i++)
    {
        RegisteredPlugin *registered_plugin = &registered_plugins[i];
        if (registered_plugin->lifetime != PLUGIN_LIFETIME_UNKNOWN)
        {
            continue;
        }

        PluginLifetime requested_lifetime = PLUGIN_LIFETIME_UNKNOWN;

        for (int j = 0; j < GET_ARRAY_LENGTH(requested_plugins_explicit); j++)
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
            if (!is_lifetime_supported(registered_plugin->metadata, requested_lifetime))
            {
                LOG_ERR("plugin '%s' - '%s' does not support requested lifetime '%d'",
                        registered_plugin->metadata->interface_name, registered_plugin->metadata->plugin_name, requested_lifetime);
                return -1;
            }

            registered_plugin->lifetime = requested_lifetime;
            continue;
        }

        if (registered_plugin->metadata->supported_lifetimes_len == 1)
        {
            registered_plugin->lifetime = registered_plugin->metadata->supported_lifetimes[0];
            continue;
        }

        if (registered_plugin->metadata->preferred_lifetime == PLUGIN_LIFETIME_UNKNOWN)
        {
            continue;
        }

        registered_plugin_set_preferred_lifetime(registered_plugin);
    }
    return 0;
}

int32_t calculate_plugin_metadata_initialization_order(
    const LoggerInterface *logger,
    const RegisteredPlugin *registered_plugins, size_t registered_plugins_len)
{
    (void)logger, registered_plugins, registered_plugins_len;
    TODO("Topologically sort the registered")
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

    const LoggerInterface *logger = context->logger;

    RequestedPlugin requested_plugins[MAX_REGISTERED_PLUGINS_LEN] = {0};
    size_t requested_plugins_len = 0;
    ret = seed_explicitly_requested_plugins(context, requested_plugins_explicit, requested_plugins, &requested_plugins_len);
    if (ret < 0)
    {
        LOG_ERR("Unable to seed requested plugins: %d", ret);
        return ret;
    }

    ret = load_requested_plugins(
        logger,
        plugin_registry,
        static_plugin_metadatas,
        requested_plugins, requested_plugins_len,
        context->registered_plugins, &context->registered_plugins_len);

    ret = resolve_registered_plugins_lifetimes(
        logger,
        requested_plugins_explicit,
        context->registered_plugins, context->registered_plugins_len);

    TODO("Loop through all singleton registered plugins and add them to singleton scope after a topological sort (if theyre not yet added)");

    ret = calculate_plugin_metadata_initialization_order(logger, context->registered_plugins, context->registered_plugins_len);
    TODO("Do topological sort and figure out lifetimes. Initialize the singleton dependencies right away")
    TODO("Make scopes work (create singleton scope)")
    TODO("Add singletons based on requested plugins")
    TODO("Of plugins with unknown scope, create singletons for the ones that have it as preferred lifetime")

    return 0;
}