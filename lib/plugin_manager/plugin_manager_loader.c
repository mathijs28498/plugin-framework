#include "plugin_manager_loader.h"

#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>
#include <stdio.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(plugin_manager_loader, LOG_LEVEL_DEBUG)
#include <plugin_utils.h>

#include "plugin_manager_types.h"
#include "plugin_manager.h"
#include "plugin_registry.h"

TODO("Check if this algorithm can/should be made better/faster")
int32_t resolve_requested_plugins_dynamic(
    const LoggerInterface *logger,
    RequestedPlugin *requested_plugins,
    size_t requested_plugins_len,
    const PluginRegistry *plugin_registry,
    PluginModule *plugin_modules,
    size_t *plugin_modules_len)
{
    for (size_t i = 0; i < requested_plugins_len; i++)
    {
        RequestedPlugin *requested_plugin = &requested_plugins[i];
        bool use_default = strlen(requested_plugin->plugin_name) == 0;

        const PluginDefinition *plugin_definition_to_add = NULL;
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
                if (strcmp(plugin_name, plugin_definition->plugin_name) == 0)
                {
                    plugin_definition_to_add = plugin_definition;
                    break;
                }
            }

            if (plugin_definition_to_add != NULL)
            {
                break;
            }
        }

        if (plugin_definition_to_add == NULL)
        {
            LOG_DBG(logger, "Interface '%s' not found in registry, looking for internal plugin", requested_plugin->interface_name);
            continue;
        }

        PluginModule *plugin_module = &plugin_modules[*plugin_modules_len];

        snprintf(plugin_module->interface_name, sizeof(plugin_module->interface_name),
                 "%s", requested_plugin->interface_name);
        plugin_module->plugin_name = plugin_definition_to_add->plugin_name;
        plugin_module->plugin_path = plugin_definition_to_add->module_path;
        plugin_module->is_explicit = requested_plugin->is_explicit;
        (*plugin_modules_len)++;
        TODO("Add max index check here")

        requested_plugin->is_resolved = true;
    }

    return 0;
}

#define GET_FUNCTION_PROC(handle, function_type, prefix, midfix, postfix, function)                                                         \
    do                                                                                                                                      \
    {                                                                                                                                       \
        char function_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN + sizeof(midfix) + PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN]; \
        snprintf(function_name, sizeof(function_name), "%s%s%s", (prefix), (midfix), (postfix));                                            \
                                                                                                                                            \
        (function) = (function_type)GetProcAddress((handle), function_name);                                                                \
    } while (0)

int32_t load_plugin_modules(
    const LoggerInterface *logger,
    const PluginModule *plugin_modules,
    size_t plugin_modules_len,
    PluginProvider *plugin_providers,
    size_t *plugin_providers_len)
{
    for (size_t i = 0; i < plugin_modules_len; i++)
    {
        const PluginModule *plugin_module = &plugin_modules[i];
        PluginProvider *plugin_provider = &plugin_providers[*plugin_providers_len];
        (*plugin_providers_len)++;
        TODO("Add max size check, figure out how to use this pattern to use a max len as well or something");

        plugin_provider->dependencies_len = 0;
        snprintf(plugin_provider->interface_name, PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN,
                 "%s", plugin_module->interface_name);
        snprintf(plugin_provider->plugin_name, PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN,
                 "%s", plugin_module->plugin_name);
        plugin_provider->is_initialized = 0;
        plugin_provider->is_explicit = plugin_module->is_explicit;

        HMODULE handle = LoadLibrary(plugin_module->plugin_path);
        if (!handle)
        {
            LOG_ERR(logger, "Failed to load plugin '%s' at '%s'", plugin_module->plugin_name, plugin_module->plugin_path);
            return -1;
        }

        PluginGetDependencies_Fn get_dependencies_proc;
        GET_FUNCTION_PROC(handle, PluginGetDependencies_Fn, plugin_module->interface_name, "_get_dependencies", "",
                          get_dependencies_proc);

        if (get_dependencies_proc)
        {
            char **dependencies;
            get_dependencies_proc(&dependencies, &plugin_provider->dependencies_len);
            for (size_t j = 0; j < plugin_provider->dependencies_len; j++)
            {
                snprintf(plugin_provider->dependencies[j].interface_name, sizeof(plugin_provider->dependencies[j].interface_name),
                         "%s", dependencies[j]);
                plugin_provider->dependencies[j].is_resolved = false;

                GET_FUNCTION_PROC(handle, PluginSetDependency_Fn, plugin_module->interface_name, "_set_", dependencies[j],
                                  plugin_provider->dependencies[j].set);
                if (!plugin_provider->dependencies[j].set)
                {
                    LOG_ERR(logger, "Could not find dependency setter '%s_set_%s'", plugin_module->interface_name, dependencies[j]);
                    return -1;
                }
            }
        }

        GET_FUNCTION_PROC(handle, PluginGetInterface_Fn, plugin_module->interface_name, "_get_interface", "",
                          plugin_provider->get_interface);
        if (!plugin_provider->get_interface)
        {
            LOG_ERR(logger, "no get interface method found for plugin: '%s' - '%s'",  plugin_module->interface_name, plugin_module->plugin_name);
            return -1;
        }

        GET_FUNCTION_PROC(handle, PluginInit_Fn, plugin_module->interface_name, "_init", "",
                          plugin_provider->init);

        GET_FUNCTION_PROC(handle, PluginShutdown_Fn, plugin_module->interface_name, "_shutdown", "",
                          plugin_provider->shutdown);
    }

    return 0;
}

int32_t resolve_requested_plugins_internal(
    const LoggerInterface *logger,
    RequestedPlugin *requested_plugins,
    size_t requested_plugins_len,
    const PluginProvider *internal_plugins,
    size_t internal_plugins_len,
    PluginProvider *plugin_providers,
    size_t *plugin_providers_len)
{
    for (size_t i = 0; i < requested_plugins_len; i++)
    {
        const RequestedPlugin *requested_plugin = &requested_plugins[i];
        if (requested_plugin->is_resolved)
        {
            continue;
        }

        bool use_default = strlen(requested_plugin->plugin_name) == 0;

        int32_t internal_plugin_index = -1;
        for (uint32_t j = 0; j < internal_plugins_len; j++)
        {
            const PluginProvider *internal_plugin = &internal_plugins[j];

            const char *internal_plugin_name = use_default
                                                   ? internal_plugin->interface_name
                                                   : internal_plugin->plugin_name;

            const char *plugin_to_add_name = use_default
                                                 ? requested_plugin->interface_name
                                                 : requested_plugin->plugin_name;

            if (strcmp(internal_plugin_name, plugin_to_add_name) == 0)
            {
                internal_plugin_index = (int32_t)j;

                TODO("See if this can be done better")
                PluginProvider *plugin_provider = &plugin_providers[*plugin_providers_len];
                memcpy(plugin_provider, internal_plugin, sizeof(*plugin_provider));
                plugin_provider->is_explicit = requested_plugin->is_explicit;
                (*plugin_providers_len)++;
                TODO("Check max length")
                break;
            }
        }

        if (internal_plugin_index < 0)
        {
            LOG_ERR(logger, "Interface '%s' not found as registered or internal plugin", requested_plugin->interface_name);
            return -1;
        }
    }

    return 0;
}

int32_t resolve_plugin_provider_dependencies(
    const LoggerInterface *logger,
    PluginProvider *plugin_providers,
    size_t plugin_providers_len,
    RequestedPlugin *requested_plugins,
    size_t *requested_plugins_len)
{
    int32_t ret;
    for (size_t i = 0; i < plugin_providers_len; i++)
    {
        PluginProvider *plugin_provider = &plugin_providers[i];

        for (uint32_t j = 0; j < plugin_provider->dependencies_len; j++)
        {
            PluginDependency *dependency = &plugin_provider->dependencies[j];
            if (dependency->is_resolved)
            {
                continue;
            }

            for (uint32_t k = 0; k < plugin_providers_len; k++)
            {
                TODO("Check if dependent is the right term here")
                const PluginProvider *dependent_plugin_provider = &plugin_providers[k];
                if (strcmp(dependency->interface_name, dependent_plugin_provider->interface_name) != 0)
                {
                    continue;
                }

                dependency->set(plugin_provider->get_interface()->context, dependent_plugin_provider->get_interface());
                dependency->is_resolved = true;
                break;
            }

            if (dependency->is_resolved)
            {
                continue;
            }

            // Adding dependency to requested plugins as it is not found yet

            // First check if dependency has already been reqeusted in the meantime
            bool to_request = true;
            for (size_t k = 0; k < *requested_plugins_len; k++)
            {
                if (strcmp(requested_plugins[k].interface_name, dependency->interface_name) == 0)
                {
                    to_request = false;
                    break;
                }
            }

            if (!to_request)
            {
                continue;
            }

            LOG_DBG(logger, "Dependency '%s' not found, adding to requested plugins", dependency->interface_name);
            ret = plugin_manager_request_plugin(logger, dependency->interface_name, NULL, false, requested_plugins, requested_plugins_len);
            if (ret < 0)
            {
                LOG_ERR(logger, "Unable to implicitly add dependency interface '%s'", dependency->interface_name);
                return ret;
            }
        }
    }

    return 0;
}

bool plugin_provider_depends_on_interface(
    const PluginProvider *dependent_plugin_provider,
    const char *interface_name)
{
    for (uint32_t i = 0; i < dependent_plugin_provider->dependencies_len; i++)
    {
        if (strcmp(dependent_plugin_provider->dependencies[i].interface_name, interface_name) == 0)
        {
            return true;
        }
    }

    return false;
}

int32_t calculate_plugin_provider_initialization_order(
    const LoggerInterface *logger,
    const PluginProvider *plugin_providers,
    size_t plugin_providers_len,
    size_t *sorted_plugin_providers_indices)
{
    uint32_t indegrees[PLUGIN_MANAGER_MAX_PLUGINS_LEN] = {0};
    uint32_t queue_head = 0;
    uint32_t queue_tail = 0;

    // Calculate initial indegrees
    for (int i = 0; i < plugin_providers_len; i++)
    {
        const PluginProvider *plugin_provider = &plugin_providers[i];
        uint32_t *indegree = &indegrees[i];
        for (uint32_t j = 0; j < plugin_provider->dependencies_len; j++)
        {
            (*indegree)++;
        }
        LOG_DBG(logger, "'%s %s' - indegree: %d", plugin_provider->interface_name, plugin_provider->plugin_name, *indegree);
    }

    // Seed initial indices
    for (int i = 0; i < plugin_providers_len; i++)
    {
        if (indegrees[i] == 0)
        {
            sorted_plugin_providers_indices[queue_tail] = i;
            queue_tail++;
        }
    }

    // Iteratively fill sorted indices
    SAFE_WHILE(
        queue_head != queue_tail,
        PLUGIN_MANAGER_MAX_PLUGINS_LEN + 1,
        {
            LOG_ERR(logger, "Plugin initialization topological sort exceeded max iterations");
            return -1;
        })
    {
        size_t current_idx = sorted_plugin_providers_indices[queue_head];
        queue_head++;
        const PluginProvider *current_plugin_provider = &plugin_providers[current_idx];

        for (int i = 0; i < plugin_providers_len; i++)
        {
            if (indegrees[i] == 0)
            {
                continue;
            }

            TODO("Check if dependent is the right term here")
            const PluginProvider *dependent_plugin_provider = &plugin_providers[i];
            if (plugin_provider_depends_on_interface(
                    dependent_plugin_provider,
                    current_plugin_provider->interface_name))
            {
                indegrees[i]--;

                if (indegrees[i] == 0)
                {
                    if (queue_tail >= plugin_providers_len)
                    {
                        LOG_ERR(logger, "Queue overflow: Logic error in dependency graph.");
                        return -1;
                    }
                    sorted_plugin_providers_indices[queue_tail] = i;
                    queue_tail++;
                }
            }
        }
    }

    if (queue_tail != plugin_providers_len)
    {
        LOG_ERR(logger, "Cyclic dependency detected! Initialized %d out of %d plugins.", queue_tail, plugin_providers_len);
        return -1;
    }

    return 0;
}

int32_t initialize_plugins(
    const struct LoggerInterface *logger,
    const size_t *sorted_plugin_providers_indices,
    const PluginProvider *plugin_providers,
    size_t plugin_providers_len,
    InterfaceInstance *interface_instances,
    size_t *interface_instances_len)
{
    for (size_t i = 0; i < plugin_providers_len; i++)
    {
        const PluginProvider *plugin_provider = &plugin_providers[sorted_plugin_providers_indices[i]];

        InterfaceInstance *interface_instance = &interface_instances[*interface_instances_len];
        (*interface_instances_len)++;
        snprintf(interface_instance->interface_name, sizeof(interface_instance->interface_name),
                 "%s", plugin_provider->interface_name);
        interface_instance->iface = plugin_provider->get_interface();
        interface_instance->is_explicit = plugin_provider->is_explicit;

        if (plugin_provider->is_initialized || !plugin_provider->init)
        {
            continue;
        }
        int32_t init_ret = plugin_provider->init(interface_instance->iface->context);
        if (init_ret < 0)
        {
            LOG_ERR(logger, "Unable to init interface '%s'-'%s': %d",
                    plugin_provider->interface_name,
                    plugin_provider->plugin_name,
                    init_ret);
            return init_ret;
        }
    }

    return 0;
}