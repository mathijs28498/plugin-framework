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

        plugin_module->handle = LoadLibrary(plugin_module->plugin_definition->path);
        if (!plugin_module->handle)
        {
            LOG_ERR(logger_api, "Failed to load plugin \"%s\" at \"%s\"", plugin_module->plugin_definition->plugin_name, plugin_module->plugin_definition->path);
            continue;
        }

        static const char get_dependencies_postfix[] = "_get_dependencies";
        char get_dependencies_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN + sizeof(get_dependencies_postfix)];

        snprintf(get_dependencies_function_name, sizeof(get_dependencies_function_name),
                 "%s%s", plugin_module->plugin_definition->api_name, get_dependencies_postfix);

        FARPROC get_dependencies_proc = GetProcAddress(plugin_module->handle, get_dependencies_function_name);
        if (get_dependencies_proc)
        {
            // TODO: Make these functions casted to a function signature typedef
            char **dependencies;
            get_dependencies_proc(&dependencies, &plugin_module->dependencies_len);
            for (size_t j = 0; j < plugin_module->dependencies_len; j++)
            {
                plugin_module->dependencies[j].api_name = dependencies[j];
                plugin_module->dependencies[j].resolved = false;
            }
        }

        static const char get_api_postfix[] = "_get_api";
        char get_api_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN + sizeof(get_api_postfix)];

        snprintf(get_api_function_name, sizeof(get_api_function_name),
                 "%s%s", plugin_module->plugin_definition->api_name, get_api_postfix);

        FARPROC get_api_proc = GetProcAddress(plugin_module->handle, get_api_function_name);
        if (get_api_proc)
        {
            // TODO: Make these functions casted to a function signature typedef
            // TODO: Remove this call, as api here should not be needed
            plugin_module->api = (PluginManagerBaseApi *)get_api_proc();

            api_instances[*api_instances_len].api = (PluginManagerBaseApi *)get_api_proc();
            snprintf(api_instances[*api_instances_len].api_name, PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN,
                     // TODO: make this name to api_name
                     "%s", plugin_module->plugin_definition->api_name);
            (*api_instances_len)++;
        }
        else
        {
            LOG_ERR(logger_api, "no api found for plugin: %s", plugin_module->plugin_definition->plugin_name);
        }
    }

    return 0;
}

int32_t resolve_requested_plugins_internal(
    const LoggerApi *logger_api,
    RequestedPlugin *requested_plugins,
    size_t requested_plugins_len,
    const PluginStatic *internal_plugins,
    size_t internal_plugins_len,
    // PluginStatic *static_plugins_list,
    // size_t *static_plugins_len,
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
        // TODO: Add max index check here
    }

    return 0;
}

// int32_t resolve_plugin_module_dependencies(
//     const struct LoggerApi *logger_api,
//     PluginModule *plugin_modules,
//     size_t plugin_modules_len)
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
            // TODO: Handle when dependency is not found (add it from the registry if it is available)
            PluginDependency *dependency = &plugin_module->dependencies[j];

            for (uint32_t k = 0; k < api_instances_len; k++)
            {
                // TODO: look into the api_instances, not plugin_modules
                const ApiInstance *api_instance = &api_instances[k];
                if (strcmp(api_instance->api_name, dependency->api_name) != 0)
                {
                    continue;
                }

                static const char set_dependency_midfix[] = "_set_";
                // TODO: Fix the sizing here
                char set_dependency_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN + sizeof(set_dependency_midfix)];

                snprintf(set_dependency_function_name, sizeof(set_dependency_function_name),
                         "%s%s%s", plugin_module->plugin_definition->api_name, set_dependency_midfix, dependency->api_name);

                FARPROC set_dependency_proc = GetProcAddress(plugin_module->handle, set_dependency_function_name);
                if (!set_dependency_proc)
                {
                    LOG_ERR(logger_api, "%s() not found", set_dependency_function_name);
                    return -1;
                }

                // TODO: Make these functions casted to a function signature typedef
                set_dependency_proc(plugin_module->api->context, api_instance->api);
                dependency->resolved = true;
            }

            if (dependency->resolved)
            {
                continue;
            }

            // TODO: Make this a method that can be reused by the plugin_manager_add
            bool to_request = true;
            for (size_t l = 0; l < *requested_plugins_len; l++)
            {
                if (strcmp(requested_plugins[l].api_name, dependency->api_name) == 0)
                {
                    to_request = false;
                    break;
                }
            }

            if (!to_request)
            {
                continue;
            }

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

int32_t initialize_plugins(
    const struct LoggerApi *logger_api,
    PluginModule *plugin_modules,
    size_t plugin_modules_len)
{
    for (size_t i = 0; i < plugin_modules_len; i++)
    {
        PluginModule *plugin_module = &plugin_modules[i];
        static const char init_postfix[] = "_init";
        // TODO: Fix the sizing here
        char init_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN + sizeof(init_postfix)];
        snprintf(init_function_name, sizeof(init_function_name),
                 "%s%s", plugin_module->plugin_definition->api_name, init_postfix);

        FARPROC init_proc = GetProcAddress(plugin_module->handle, init_function_name);
        if (init_proc)
        {
            // TODO: Make these functions casted to a function signature typedef
            int32_t init_ret = (int32_t)init_proc(plugin_module->api->context);
            // TODO: Do something with the return value;
            (void)init_ret;
        }
        else
        {
            LOG_ERR(logger_api, "%s() not found\n", init_function_name);
        }
    }

    return 0;
}