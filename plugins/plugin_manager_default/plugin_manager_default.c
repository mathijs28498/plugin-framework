#include "plugin_manager_default.h"
#include <plugin_utils.h>

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
TODO("Hide this in a separate place behind a define to make it platform independent")
#include <Windows.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(logger_console, LOG_LEVEL_DEBUG);
#include <plugin_manager_pm_interface.h>
#include <plugin_sdk_types.h>

#include "plugin_manager_default_register.h"

TODO("Figure out where these belong")
#define PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN 256
#define PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH 256
TODO("Figure out where to put this")
#define MAX_REGISTERED_PLUGINS 64

int32_t resolve_requested_plugins(
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
            TODO("Write error message");
            return -1;
        }
    }

    return 0;
}

int32_t resolve_get_metadata_fn_dynamic(const char *module_path, const char *target_name, PluginGetMetadata_Fn *out_get_metadata_fn)
{
    assert(out_get_metadata_fn != NULL);

    TODO("Figure out how to destroy the handle if that is necessary, also allow this code to be optionally compiled based on platform/PLUGIN_BUILD_SHARED")
    HMODULE handle = LoadLibrary(module_path);
    if (!handle)
    {
        TODO("Add error log here")
        return -1;
    }

    char function_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
    snprintf(function_name, sizeof(function_name), "%s_get_plugin_metadata", target_name);

    FARPROC proc_address = GetProcAddress(handle, function_name);

    if (!proc_address)
    {
        // TODO: Add error log here (Symbol not found)
        TODO("Add error log here")
        FreeLibrary(handle);
        return -2;
    }

    *out_get_metadata_fn = (PluginGetMetadata_Fn)proc_address;

    return 0;
}

int32_t resolve_plugin_metadatas(
    const PluginDefinition **plugin_definitions, size_t plugin_definitions_len,
    const PluginMetadata **static_plugin_metadatas, size_t static_plugin_metadatas_len,
    const PluginMetadata **out_plugin_metadatas, size_t *out_plugin_metadatas_len)
{
    int32_t ret;
    for (size_t i = 0; i < plugin_definitions_len; i++)
    {
        const PluginDefinition *plugin_definition = plugin_definitions[i];
        bool static_metadata_found = false;

        for (size_t j = 0; j < static_plugin_metadatas_len; j++)
        {
            const PluginMetadata *static_plugin_metadata = static_plugin_metadatas[j];
            if (strcmp(plugin_definition->interface_name, static_plugin_metadata->interface_name) == 0)
            {
                out_plugin_metadatas[*out_plugin_metadatas_len] = static_plugin_metadata;
                (*out_plugin_metadatas_len)++;
                TODO("Add dbg text here")
                TODO("Add max size check here")
                static_metadata_found = true;
                break;
            }
        }

        if (static_metadata_found)
        {
            continue;
        }

        PluginGetMetadata_Fn get_metadata_fn = NULL;
        ret = resolve_get_metadata_fn_dynamic(plugin_definition->module_path, plugin_definition->target_name, &get_metadata_fn);
        if (ret < 0)
        {
            TODO("Add error log here")
            return ret;
        }
        const PluginMetadata *plugin_metadata = get_metadata_fn();
        if (plugin_metadata == NULL)
        {
            TODO("Add error log here")
            return -1;
        }

        out_plugin_metadatas[*out_plugin_metadatas_len] = plugin_metadata;
        (*out_plugin_metadatas_len)++;
        TODO("Add dbg text here")
        TODO("Add max size check here")
    }

    return 0;
}

int32_t resolve_plugin_metadata_dependencies(
    const PluginMetadata **plugin_metadatas, size_t plugin_metadatas_len, size_t resolved_plugin_metadata_offset,
    RequestedPlugin *out_requested_plugins, size_t *out_requested_plugins_len)
{
    for (size_t i = resolved_plugin_metadata_offset; i < plugin_metadatas_len; i++)
    {
        const PluginMetadata *plugin_metadata = plugin_metadatas[i];
        for (size_t j = 0; j < plugin_metadata->dependencies_len; j++)
        {
            const PluginDependency *plugin_dependency = &plugin_metadata->dependencies[j];

            bool dependency_already_present = false;

            for (size_t k = 0; k < plugin_metadatas_len; k++)
            {
                const PluginMetadata *plugin_metadata_to_check = plugin_metadatas[k];
                if (strcmp(plugin_dependency->interface_name, plugin_metadata_to_check->interface_name) == 0)
                {
                    dependency_already_present = true;
                    break;
                }
            }

            if (dependency_already_present)
            {
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
                continue;
            }

            // The dependency is not plugin metadatas, and is not already requested, now we will request it
            out_requested_plugins[*out_requested_plugins_len].interface_name = plugin_dependency->interface_name;
            out_requested_plugins[*out_requested_plugins_len].plugin_name = "";
            (*out_requested_plugins_len)++;
            TODO("Add dbg text here")
            TODO("Add max size check here")
        }
    }
    return 0;
}
int32_t load_requested_plugins(
    const PluginRegistry *plugin_registry,
    const PluginMetadata **static_plugin_metadatas, size_t static_plugin_metadatas_len,
    RequestedPlugin *requested_plugins, size_t requested_plugins_len,
    const PluginMetadata **out_plugin_metadatas, size_t *out_plugin_metadatas_len)
{
    int ret;
    size_t resolved_plugin_metadata_offset = *out_plugin_metadatas_len;

    SAFE_WHILE(
        requested_plugins_len > 0,
        PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH,
        {
            TODO("Add error log")
            return -1;
        })
    {
        const PluginDefinition *plugin_definitions[MAX_REGISTERED_PLUGINS];
        size_t plugin_definitions_len = 0;

        TODO("Add error handling messages")
        ret = resolve_requested_plugins(
            requested_plugins, requested_plugins_len,
            plugin_registry,
            plugin_definitions, &plugin_definitions_len);

        ret = resolve_plugin_metadatas(
            plugin_definitions, plugin_definitions_len,
            static_plugin_metadatas, static_plugin_metadatas_len,
            out_plugin_metadatas, out_plugin_metadatas_len);

        requested_plugins_len = 0;

        ret = resolve_plugin_metadata_dependencies(
            out_plugin_metadatas, *out_plugin_metadatas_len, resolved_plugin_metadata_offset,
            requested_plugins, &requested_plugins_len);

        resolved_plugin_metadata_offset = *out_plugin_metadatas_len;
    }

    return 0;
}

int32_t plugin_manager_default_bootstrap(
    PluginManagerContext *context,
    int argc, char **argv, void *platform_context,
    const PluginRegistry *plugin_registry,
    const PluginMetadata **static_plugin_metadatas, size_t static_plugin_metadatas_len,
    const RequestedPlugin *explicitly_requested_plugins, size_t explicitly_requested_plugins_len)
{
    int32_t ret;
    TODO("Call the environment set_arguments");
    (void)context, argc, argv, platform_context;

    const PluginMetadata *plugin_metadatas[MAX_REGISTERED_PLUGINS];
    size_t plugin_metadatas_len = 0;

    TODO("Check if should allow to have the plugin_manager requested")
    RequestedPlugin requested_plugins[MAX_REGISTERED_PLUGINS] = {0};
    size_t requested_plugins_len = 0;
    for (size_t i = 0; i < explicitly_requested_plugins_len; i++)
    {
        const RequestedPlugin *requested_plugin = &explicitly_requested_plugins[i];
        if (strcmp("plugin_manager", requested_plugin->interface_name) == 0)
        {
            requested_plugins[0].interface_name = requested_plugin->interface_name;
            requested_plugins[0].plugin_name = requested_plugin->plugin_name;
            requested_plugins_len++;
            break;
        }
    }
    if (requested_plugins_len == 0)
    {
        requested_plugins[0].interface_name = "plugin_manager";
        requested_plugins[0].plugin_name = "";
        requested_plugins_len++;
    }

    TODO("Somehow make sure that these always get to singleton and that plugin_manager doesnt get its context created")
    ret = load_requested_plugins(
        plugin_registry,
        static_plugin_metadatas, static_plugin_metadatas_len,
        requested_plugins, requested_plugins_len,
        plugin_metadatas, &plugin_metadatas_len);
    
    TODO("Set the logger and environment in the context")

    requested_plugins_len = 0;
    for (size_t i = 0; i < explicitly_requested_plugins_len; i++)
    {
        const RequestedPlugin *explicitly_requested_plugin = &explicitly_requested_plugins[i];
        bool plugin_already_loaded = false;
        for (size_t j = 0; j < plugin_metadatas_len; j++)
        {
            const PluginMetadata *plugin_metadata = plugin_metadatas[j];
            if (strcmp(explicitly_requested_plugin->interface_name, plugin_metadata->interface_name) == 0)
            {
                plugin_already_loaded = true;
                break;
            }
        }

        if (plugin_already_loaded)
        {
            continue;
        }

        requested_plugins[requested_plugins_len].interface_name = explicitly_requested_plugin->interface_name;
        requested_plugins[requested_plugins_len].plugin_name = explicitly_requested_plugin->plugin_name;
        requested_plugins_len++;
    }

    ret = load_requested_plugins(
        plugin_registry,
        static_plugin_metadatas, static_plugin_metadatas_len,
        requested_plugins, requested_plugins_len,
        plugin_metadatas, &plugin_metadatas_len);

    TODO("Do topological sort and figure out lifetimes. Initialize the singleton dependencies right away")
    TODO("Make scopes work (create singleton scope)")

    return 0;
}

int32_t plugin_manager_default_get_singleton(PluginManagerContext *context, const char *interface_name, void **iface)
{
    assert(iface != NULL);
    (void)context, interface_name, iface;
    return 0;
    // return NOT_IMPLEMENTED(int32_t, context, interface_name, iface);
}