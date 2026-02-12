#include "plugin_manager_loader.h"

#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>
#include <stdio.h>

#include <logger_api.h>

#include "plugin_manager_types.h"

#define LOGGER_API_TAG "plugin_manager_loader"

// TODO: Check if this algorithm can/should be made better/faster
int32_t resolve_requested_plugins_registry(
    const LoggerApi *logger_api,
    RequestedPlugin *requested_plugins,
    size_t requested_plugins_len,
    const PluginRegistry *plugin_registry,
    PluginModule *plugin_modules,
    size_t *plugin_modules_len)
{
    for (size_t i = 0; i < requested_plugins_len; i++)
    {
        const RequestedPlugin *requested_plugin = &requested_plugins[i];

        bool use_default = strlen(requested_plugin->plugin_name) == 0;

        int32_t plugin_definition_index = -1;
        for (uint32_t j = 0; j < plugin_registry->plugin_definitions_len; j++)
        {
            const PluginDefinition *plugin_definition = &plugin_registry->plugin_definitions[j];

            const char *plugin_info_name = use_default
                                               ? plugin_definition->api_name
                                               : plugin_definition->plugin_name;

            const char *plugin_to_add_name = use_default
                                                 ? requested_plugin->api_name
                                                 : requested_plugin->plugin_name;

            if (strcmp(plugin_info_name, plugin_to_add_name) == 0)
            {
                plugin_definition_index = (int32_t)j;
                break;
            }
        }

        if (plugin_definition_index < 0)
        {
            LOG_DBG(logger_api, "Plugin not found in registry, looking for internal plugin \"%s\"", requested_plugin->api_name);
            continue;
        }

        plugin_modules[*plugin_modules_len].plugin_definition = &plugin_registry->plugin_definitions[plugin_definition_index];
        plugin_modules[*plugin_modules_len].dependencies_len = 0;
        (*plugin_modules_len)++;
        requested_plugins[i].resolved = true;
        // TODO: Add max index check here
    }

    return 0;
}

int32_t load_plugin_modules(
    const LoggerApi *logger_api,
    PluginModule *plugin_modules,
    size_t plugin_modules_len,
    ApiInstance *api_instances,
    size_t *api_instances_len)
{
    for (size_t i = 0; i < plugin_modules_len; i++)
    {
        PluginModule *plugin_module = &plugin_modules[i];

        HMODULE handle = LoadLibrary(plugin_module->plugin_definition->path);
        if (!handle)
        {
            LOG_ERR(logger_api, "Failed to load plugin \"%s\" at \"%s\"", plugin_module->plugin_definition->plugin_name, plugin_module->plugin_definition->path);
            return -1;
        }

        // Register set_dependency functions
        static const char get_dependencies_postfix[] = "_get_dependencies";
        char get_dependencies_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN + sizeof(get_dependencies_postfix)];
        snprintf(get_dependencies_function_name, sizeof(get_dependencies_function_name),
                 "%s%s", plugin_module->plugin_definition->api_name, get_dependencies_postfix);

        PluginGetDependencies_Fn get_dependencies_proc = (PluginGetDependencies_Fn)GetProcAddress(handle, get_dependencies_function_name);
        if (get_dependencies_proc)
        {
            char **dependencies;
            get_dependencies_proc(&dependencies, &plugin_module->dependencies_len);
            for (size_t j = 0; j < plugin_module->dependencies_len; j++)
            {
                static const char set_dependency_midfix[] = "_set_";
                char set_dependency_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN + sizeof(set_dependency_midfix)];

                snprintf(set_dependency_function_name, sizeof(set_dependency_function_name),
                         "%s%s%s", plugin_module->plugin_definition->api_name, set_dependency_midfix, dependencies[j]);
                PluginSetDependency_Fn set_dependency_proc = (PluginSetDependency_Fn)GetProcAddress(handle, set_dependency_function_name);
                if (!set_dependency_proc)
                {
                    LOG_ERR(logger_api, "Could not find dependency setter '%s'", set_dependency_function_name);
                    return -1;
                }

                plugin_module->dependencies[j].api_name = dependencies[j];
                plugin_module->dependencies[j].resolved = false;
                plugin_module->dependencies[j].set = set_dependency_proc;
            }
        }

        // Register get_api function
        static const char get_api_postfix[] = "_get_api";
        char get_api_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN + sizeof(get_api_postfix)];
        snprintf(get_api_function_name, sizeof(get_api_function_name),
                 "%s%s", plugin_module->plugin_definition->api_name, get_api_postfix);

        PluginGetApi_Fn get_api_proc = (PluginGetApi_Fn)GetProcAddress(handle, get_api_function_name);
        if (!get_api_proc)
        {
            LOG_ERR(logger_api, "no api found for plugin: %s", plugin_module->plugin_definition->plugin_name);
            return -1;
        }
        plugin_module->get_api = get_api_proc;

        // Register init function
        static const char init_postfix[] = "_init";
        char init_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN + sizeof(init_postfix)];
        snprintf(init_function_name, sizeof(init_function_name),
                 "%s%s", plugin_module->plugin_definition->api_name, init_postfix);

        PluginInit_Fn init_proc = (PluginInit_Fn)GetProcAddress(handle, init_function_name);

        if (init_proc)
        {
            plugin_module->init = init_proc;
        }

        // Setting api_instance values
        api_instances[*api_instances_len].api = get_api_proc();
        snprintf(api_instances[*api_instances_len].api_name, PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN,
                 "%s", plugin_module->plugin_definition->api_name);
        (*api_instances_len)++;
    }

    return 0;
}

int32_t resolve_requested_plugins_internal(
    const LoggerApi *logger_api,
    RequestedPlugin *requested_plugins,
    size_t requested_plugins_len,
    const PluginStatic *internal_plugins,
    size_t internal_plugins_len,
    ApiInstance *api_instances,
    size_t *api_instances_len)
{
    for (size_t i = 0; i < requested_plugins_len; i++)
    {
        const RequestedPlugin *requested_plugin = &requested_plugins[i];
        if (requested_plugin->resolved)
        {
            continue;
        }

        bool use_default = strlen(requested_plugin->plugin_name) == 0;

        int32_t internal_plugin_index = -1;
        for (uint32_t j = 0; j < internal_plugins_len; j++)
        {
            const PluginStatic *internal_plugin = &internal_plugins[j];

            const char *internal_plugin_name = use_default
                                                   ? internal_plugin->api_name
                                                   : internal_plugin->plugin_name;

            const char *plugin_to_add_name = use_default
                                                 ? requested_plugin->api_name
                                                 : requested_plugin->plugin_name;

            if (strcmp(internal_plugin_name, plugin_to_add_name) == 0)
            {
                internal_plugin_index = (int32_t)j;
                break;
            }
        }

        if (internal_plugin_index < 0)
        {
            LOG_ERR(logger_api, "Plugin not found as internal plugin \"%s\"", requested_plugin->api_name);
            return -1;
        }

        api_instances[*api_instances_len].api = internal_plugins[internal_plugin_index].api;
        snprintf(api_instances[*api_instances_len].api_name, PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN,
                 "%s", internal_plugins[internal_plugin_index].api_name);
        (*api_instances_len)++;
        requested_plugins[i].resolved = true;
    }

    return 0;
}

int32_t resolve_plugin_module_dependencies(
    const LoggerApi *logger_api,
    const ApiInstance *api_instances,
    size_t api_instances_len,
    PluginModule *plugin_modules,
    size_t plugin_modules_len,
    RequestedPlugin *requested_plugins,
    size_t *requested_plugins_len)
{
    for (size_t i = 0; i < plugin_modules_len; i++)
    {
        PluginModule *plugin_module = &plugin_modules[i];

        for (uint32_t j = 0; j < plugin_module->dependencies_len; j++)
        {
            PluginDependency *dependency = &plugin_module->dependencies[j];

            for (uint32_t k = 0; k < api_instances_len; k++)
            {
                // TODO: look into the api_instances, not plugin_modules
                const ApiInstance *api_instance = &api_instances[k];
                if (strcmp(api_instance->api_name, dependency->api_name) != 0)
                {
                    continue;
                }

                dependency->set(plugin_module->get_api()->context, api_instance->api);
                dependency->resolved = true;
            }

            if (dependency->resolved)
            {
                continue;
            }

            // TODO: Make this a method that can be reused by the plugin_manager_add
            // Adding dependency to requested plugins as it is not found yet

            // First check if dependency has already been reqeusted in the meantime
            bool to_request = true;
            for (size_t k = 0; k < *requested_plugins_len; k++)
            {
                if (strcmp(requested_plugins[k].api_name, dependency->api_name) == 0)
                {
                    to_request = false;
                    break;
                }
            }

            if (!to_request)
            {
                continue;
            }

            // If dependency has not been requested yet, add it to requested plugins
            RequestedPlugin *requested_plugin = &requested_plugins[*requested_plugins_len];
            snprintf(requested_plugin->api_name, sizeof(requested_plugin->api_name), "%s", dependency->api_name);
            requested_plugin->plugin_name[0] = '\0';
            requested_plugin->resolved = false;
            (*requested_plugins_len)++;
            LOG_DBG(logger_api, "dependency '%s' not found, adding to requested plugins", requested_plugin->api_name);
        }
    }

    return 0;
}

bool api_is_plugin_module(
    const char *api_name,
    const PluginModule *plugin_modules,
    size_t plugin_modules_len)
{
    for (int i = 0; i < plugin_modules_len; i++)
    {
        if (strcmp(api_name, plugin_modules[i].plugin_definition->api_name) == 0)
        {
            return true;
        }
    }

    return false;
}

bool plugin_module_depends_on_api(
    const PluginModule *dependent_plugin_module,
    const char *api_name)
{
    for (uint32_t i = 0; i < dependent_plugin_module->dependencies_len; i++)
    {
        if (strcmp(dependent_plugin_module->dependencies[i].api_name, api_name) == 0)
        {
            return true;
        }
    }

    return false;
}

int32_t calculate_plugin_module_initialization_order(
    const LoggerApi *logger_api,
    const PluginModule *plugin_modules,
    size_t plugin_modules_len,
    uint32_t *sorted_plugin_modules_indices)
{
    uint32_t indegrees[PLUGIN_MANAGER_MAX_PLUGINS_LEN] = {0};
    uint32_t queue_head = 0;
    uint32_t queue_tail = 0;

    // Calculate initial indegrees
    for (int i = 0; i < plugin_modules_len; i++)
    {
        const PluginModule *plugin_module = &plugin_modules[i];
        uint32_t *indegree = &indegrees[i];
        for (uint32_t j = 0; j < plugin_module->dependencies_len; j++)
        {
            const PluginDependency *dependency = &plugin_module->dependencies[j];
            if (api_is_plugin_module(dependency->api_name, plugin_modules, plugin_modules_len))
            {
                (*indegree)++;
            }
        }
        LOG_DBG(logger_api, "'%s'-'%s' - indegree: %d", plugin_module->plugin_definition->api_name, plugin_module->plugin_definition->plugin_name, *indegree);
    }

    // Seed initial indices
    for (int i = 0; i < plugin_modules_len; i++)
    {
        if (indegrees[i] == 0)
        {
            sorted_plugin_modules_indices[queue_tail] = i;
            queue_tail++;
        }
    }

    // TODO: Add upper bound to while loop while macro
    // Iteratively fille sorted indices
    while (queue_head != queue_tail)
    {
        uint32_t current_idx = sorted_plugin_modules_indices[queue_head];
        queue_head++;
        const PluginModule *current_plugin_module = &plugin_modules[current_idx];

        for (int i = 0; i < plugin_modules_len; i++)
        {
            if (indegrees[i] == 0)
            {
                continue;
            }

            const PluginModule *dependent_plugin_module = &plugin_modules[i];
            if (plugin_module_depends_on_api(
                    dependent_plugin_module,
                    current_plugin_module->plugin_definition->api_name))
            {
                indegrees[i]--;

                if (indegrees[i] == 0)
                {
                    if (queue_tail >= plugin_modules_len)
                    {
                        LOG_ERR(logger_api, "Queue overflow: Logic error in dependency graph.");
                        return -1;
                    }
                    sorted_plugin_modules_indices[queue_tail] = i;
                    queue_tail++;
                }
            }
        }
    }

    if (queue_tail != plugin_modules_len)
    {
        // TODO: Cyclic dependency error msg
        LOG_ERR(logger_api, "Cyclic dependency detected! Initialized %d out of %d plugins.", queue_tail, plugin_modules_len);
        return -1;
    }

    return 0;
}

int32_t initialize_plugins(
    const struct LoggerApi *logger_api,
    uint32_t *sorted_plugin_modules_indices,
    PluginModule *plugin_modules,
    size_t plugin_modules_len)
{
    for (size_t i = 0; i < plugin_modules_len; i++)
    {
        PluginModule *plugin_module = &plugin_modules[sorted_plugin_modules_indices[i]];
        if (!plugin_module->init)
        {
            continue;
        }
        int32_t init_ret = plugin_module->init(plugin_module->get_api()->context);
        if (init_ret < 0)
        {
            LOG_ERR(logger_api, "Unable to init api '%s'-'%s': %d",
                    plugin_module->plugin_definition->api_name,
                    plugin_module->plugin_definition->plugin_name,
                    init_ret);
            return init_ret;
        }
    }

    return 0;
}