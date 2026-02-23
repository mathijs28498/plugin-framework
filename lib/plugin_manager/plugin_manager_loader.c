#include "plugin_manager_loader.h"

#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>
#include <stdio.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(plugin_manager_loader, LOG_LEVEL_DEBUG)
#include <plugin_manager_common.h>

#include "plugin_manager_types.h"
#include "plugin_manager.h"

TODO("Check if this algorithm can/should be made better/faster")
int32_t resolve_requested_plugins_registry(
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

        int32_t plugin_definition_index = -1;
        for (uint32_t j = 0; j < plugin_registry->plugin_definitions_len; j++)
        {
            const PluginDefinition *plugin_definition = &plugin_registry->plugin_definitions[j];

            const char *plugin_definition_name = use_default
                                                     ? plugin_definition->interface_name
                                                     : plugin_definition->plugin_name;

            const char *plugin_to_add_name = use_default
                                                 ? requested_plugin->interface_name
                                                 : requested_plugin->plugin_name;

            if (strcmp(plugin_definition_name, plugin_to_add_name) == 0)
            {
                plugin_definition_index = (int32_t)j;

                PluginModule *plugin_module = &plugin_modules[*plugin_modules_len];

                plugin_module->plugin_definition = plugin_definition;
                plugin_module->dependencies_len = 0;
                plugin_module->is_explicit = requested_plugin->is_explicit;
                (*plugin_modules_len)++;
                TODO("Add max index check here")
                break;
            }
        }

        if (plugin_definition_index < 0)
        {
            LOG_DBG(logger, "Plugin '%s' not found in registry, looking for internal plugin", requested_plugin->interface_name);
            continue;
        }

        requested_plugin->resolved = true;
    }

    return 0;
}

int32_t load_plugin_modules(
    const LoggerInterface *logger,
    PluginModule *plugin_modules,
    size_t plugin_modules_len,
    InterfaceInstance *interface_instances,
    size_t *interface_instances_len)
{
    for (size_t i = 0; i < plugin_modules_len; i++)
    {
        PluginModule *plugin_module = &plugin_modules[i];

        HMODULE handle = LoadLibrary(plugin_module->plugin_definition->path);
        if (!handle)
        {
            LOG_ERR(logger, "Failed to load plugin '%s' at '%s'", plugin_module->plugin_definition->plugin_name, plugin_module->plugin_definition->path);
            return -1;
        }

        // Register set_dependency functions
        static const char get_dependencies_postfix[] = "_get_dependencies";
        char get_dependencies_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN + sizeof(get_dependencies_postfix)];
        snprintf(get_dependencies_function_name, sizeof(get_dependencies_function_name),
                 "%s%s", plugin_module->plugin_definition->interface_name, get_dependencies_postfix);

        PluginGetDependencies_Fn get_dependencies_proc = (PluginGetDependencies_Fn)GetProcAddress(handle, get_dependencies_function_name);
        if (get_dependencies_proc)
        {
            char **dependencies;
            get_dependencies_proc(&dependencies, &plugin_module->dependencies_len);
            for (size_t j = 0; j < plugin_module->dependencies_len; j++)
            {
                static const char set_dependency_midfix[] = "_set_";
                char set_dependency_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN + sizeof(set_dependency_midfix)];

                snprintf(set_dependency_function_name, sizeof(set_dependency_function_name),
                         "%s%s%s", plugin_module->plugin_definition->interface_name, set_dependency_midfix, dependencies[j]);
                PluginSetDependency_Fn set_dependency_proc = (PluginSetDependency_Fn)GetProcAddress(handle, set_dependency_function_name);
                if (!set_dependency_proc)
                {
                    LOG_ERR(logger, "Could not find dependency setter '%s'", set_dependency_function_name);
                    return -1;
                }

                plugin_module->dependencies[j].interface_name = dependencies[j];
                plugin_module->dependencies[j].resolved = false;
                plugin_module->dependencies[j].set = set_dependency_proc;
            }
        }

        // Register get_interface function
        static const char get_interface_postfix[] = "_get_interface";
        char get_interface_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN + sizeof(get_interface_postfix)];
        snprintf(get_interface_function_name, sizeof(get_interface_function_name),
                 "%s%s", plugin_module->plugin_definition->interface_name, get_interface_postfix);

        PluginGetInterface_Fn get_interface_proc = (PluginGetInterface_Fn)GetProcAddress(handle, get_interface_function_name);
        if (!get_interface_proc)
        {
            LOG_ERR(logger, "no interface method found for plugin: %s", plugin_module->plugin_definition->plugin_name);
            return -1;
        }
        plugin_module->get_interface = get_interface_proc;

        // Register init function
        static const char init_postfix[] = "_init";
        char init_function_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN + sizeof(init_postfix)];
        snprintf(init_function_name, sizeof(init_function_name),
                 "%s%s", plugin_module->plugin_definition->interface_name, init_postfix);

        PluginInit_Fn init_proc = (PluginInit_Fn)GetProcAddress(handle, init_function_name);
        plugin_module->init = init_proc;

        // Setting interface_instance values
        InterfaceInstance *interface_instance = &interface_instances[*interface_instances_len];
        interface_instance->iface = get_interface_proc();
        snprintf(interface_instance->interface_name, PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN,
                 "%s", plugin_module->plugin_definition->interface_name);
        interface_instance->is_explicit = plugin_module->is_explicit;
        (*interface_instances_len)++;
    }

    return 0;
}

int32_t resolve_requested_plugins_internal(
    const LoggerInterface *logger,
    RequestedPlugin *requested_plugins,
    size_t requested_plugins_len,
    const PluginStatic *internal_plugins,
    size_t internal_plugins_len,
    InterfaceInstance *interface_instances,
    size_t *interface_instances_len)
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
                                                   ? internal_plugin->interface_name
                                                   : internal_plugin->plugin_name;

            const char *plugin_to_add_name = use_default
                                                 ? requested_plugin->interface_name
                                                 : requested_plugin->plugin_name;

            if (strcmp(internal_plugin_name, plugin_to_add_name) == 0)
            {
                internal_plugin_index = (int32_t)j;

                InterfaceInstance *interface_instance = &interface_instances[*interface_instances_len];

                interface_instance->iface = internal_plugin->iface;
                snprintf(interface_instance->interface_name, PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN,
                         "%s", internal_plugin->interface_name);
                interface_instance->is_explicit = requested_plugin->is_explicit;
                (*interface_instances_len)++;
                requested_plugins[i].resolved = true;

                break;
            }
        }

        if (internal_plugin_index < 0)
        {
            LOG_ERR(logger, "Plugin '%s' not found as registered or internal plugin", requested_plugin->interface_name);
            return -1;
        }
    }

    return 0;
}

int32_t resolve_plugin_module_dependencies(
    const LoggerInterface *logger,
    const InterfaceInstance *interface_instances,
    size_t interface_instances_len,
    PluginModule *plugin_modules,
    size_t plugin_modules_len,
    RequestedPlugin *requested_plugins,
    size_t *requested_plugins_len)
{
    int32_t ret;
    for (size_t i = 0; i < plugin_modules_len; i++)
    {
        PluginModule *plugin_module = &plugin_modules[i];

        for (uint32_t j = 0; j < plugin_module->dependencies_len; j++)
        {
            PluginDependency *dependency = &plugin_module->dependencies[j];

            for (uint32_t k = 0; k < interface_instances_len; k++)
            {
                const InterfaceInstance *interface_instance = &interface_instances[k];
                if (strcmp(interface_instance->interface_name, dependency->interface_name) != 0)
                {
                    continue;
                }

                dependency->set(plugin_module->get_interface()->context, interface_instance->iface);
                dependency->resolved = true;
            }

            if (dependency->resolved)
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
            ret = plugin_manager_add_internal(logger, dependency->interface_name, NULL, false, requested_plugins, requested_plugins_len);
            if (ret < 0)
            {
                LOG_ERR(logger, "Unable to implicitly add dependency interface '%s'", dependency->interface_name);
                return ret;
            }
        }
    }

    return 0;
}

bool plugin_is_module(
    const char *interface_name,
    const PluginModule *plugin_modules,
    size_t plugin_modules_len)
{
    for (int i = 0; i < plugin_modules_len; i++)
    {
        if (strcmp(interface_name, plugin_modules[i].plugin_definition->interface_name) == 0)
        {
            return true;
        }
    }

    return false;
}

bool plugin_module_depends_on_interface(
    const PluginModule *dependent_plugin_module,
    const char *interface_name)
{
    for (uint32_t i = 0; i < dependent_plugin_module->dependencies_len; i++)
    {
        if (strcmp(dependent_plugin_module->dependencies[i].interface_name, interface_name) == 0)
        {
            return true;
        }
    }

    return false;
}

int32_t calculate_plugin_module_initialization_order(
    const LoggerInterface *logger,
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
            if (plugin_is_module(dependency->interface_name, plugin_modules, plugin_modules_len))
            {
                (*indegree)++;
            }
        }
        LOG_DBG(logger, "'%s'-'%s' - indegree: %d", plugin_module->plugin_definition->interface_name, plugin_module->plugin_definition->plugin_name, *indegree);
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

    TODO("Add upper bound to while loop while macro")
    // Iteratively fill sorted indices
    SAFE_WHILE(
        queue_head != queue_tail,
        PLUGIN_MANAGER_MAX_PLUGINS_LEN + 1,
        {
            LOG_ERR(logger, "Plugin initialization topological sort exceeded max iterations");
            return -1;
        })
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
            if (plugin_module_depends_on_interface(
                    dependent_plugin_module,
                    current_plugin_module->plugin_definition->interface_name))
            {
                indegrees[i]--;

                if (indegrees[i] == 0)
                {
                    if (queue_tail >= plugin_modules_len)
                    {
                        LOG_ERR(logger, "Queue overflow: Logic error in dependency graph.");
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
        LOG_ERR(logger, "Cyclic dependency detected! Initialized %d out of %d plugins.", queue_tail, plugin_modules_len);
        return -1;
    }

    return 0;
}

int32_t initialize_plugins(
    const struct LoggerInterface *logger,
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
        int32_t init_ret = plugin_module->init(plugin_module->get_interface()->context);
        if (init_ret < 0)
        {
            LOG_ERR(logger, "Unable to init interface '%s'-'%s': %d",
                    plugin_module->plugin_definition->interface_name,
                    plugin_module->plugin_definition->plugin_name,
                    init_ret);
            return init_ret;
        }
    }

    return 0;
}