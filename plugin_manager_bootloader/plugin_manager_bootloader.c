#include "plugin_manager_bootloader.h"
#include <plugin_sdk/plugin_utils.h>

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include <plugin_sdk/plugin_sdk_types.h>
#include <plugin_sdk/plugin_manager_interface.h>
#include <plugin_sdk/plugin_manager_pm_interface.h>
#include <dynamic_metadata_resolver.h>

#include "plugin_manager_bootloader_generated.h"

int32_t plugin_manager_bootloader_main(PluginManagerInterface *plugin_manager);

#if PLUGIN_BUILD_SHARED
#define PLUGIN_MANAGER_INTERFACE_NAME "plugin_manager"

int32_t get_plugin_manager_definition(const PluginRegistry *plugin_registry, const PluginDefinition **out_plugin_definition)
{
    assert(plugin_registry != NULL);
    assert(out_plugin_definition != NULL);

    for (size_t i = 0; i < plugin_registry->interface_definitions_len; i++)
    {
        const InterfaceDefinition *interface_definition = &plugin_registry->interface_definitions[i];
        if (strcmp(interface_definition->interface_name, PLUGIN_MANAGER_INTERFACE_NAME) != 0)
        {
            continue;
        }

        for (size_t j = 0; j < interface_definition->plugin_definitions_len; j++)
        {
            const PluginDefinition *plugin_definition = &interface_definition->plugin_definitions[j];

            if (strcmp(plugin_definition->plugin_name, interface_definition->default_plugin) == 0)
            {
                *out_plugin_definition = plugin_definition;
                return 0;
            }
        }
    }

    return -1;
}

int32_t get_bootloader_plugin_metadatas(
    const PluginRegistry *plugin_registry,
    const PluginMetadata *const **out_plugin_metadatas,
    const PluginMetadata **out_plugin_manager_metadata)
{
    assert(plugin_registry != NULL);
    assert(out_plugin_metadatas != NULL);
    assert(out_plugin_manager_metadata != NULL);

    int32_t ret;
    const PluginDefinition *plugin_manager_plugin_definition;
   ret = get_plugin_manager_definition(plugin_registry, &plugin_manager_plugin_definition);
    if (ret < 0)
    {
        TODO("Add error log");
        return -1;
    }

    PluginGetMetadata_Fn get_metadata_fn = NULL;
    ret = resolve_get_metadata_fn_dynamic(
        plugin_manager_plugin_definition->module_path,
        plugin_manager_plugin_definition->target_name,
        &get_metadata_fn);
    if (ret < 0)
    {
        TODO("Add error log");
        return ret;
    }

    *out_plugin_metadatas = NULL;
    *out_plugin_manager_metadata = get_metadata_fn();
    
    return 0;
}
#endif // #if PLUGIN_BUILD_SHARED

int32_t plugin_manager_bootloader_bootstrap(int argc, char **argv, void *platform_context)
{
    int32_t ret;

    const PluginRegistry *plugin_registry = get_plugin_registry();
    const RequestedPlugin *requested_plugins = get_bootloader_requested_plugins();
    const PluginMetadata *plugin_manager_metadata;
    const struct PluginMetadata *const *static_plugin_metadatas;

    ret = get_bootloader_plugin_metadatas(plugin_registry, &static_plugin_metadatas, &plugin_manager_metadata);

    if (ret < 0)
    {
        TODO("Add error log");
        return ret;
    }

    if (plugin_manager_metadata == NULL)
    {
        TODO("Add error log");
        return -1;
    }

    const PluginProvider *plugin_manager_provider = plugin_manager_metadata->provider;
    struct PluginManagerContext *plugin_manager_context = plugin_manager_provider->create_context();
    const PluginManagerPMVtable *plugin_manager_vtable = (const PluginManagerPMVtable *)plugin_manager_provider->vtable;

    if (plugin_manager_provider->init != NULL)
    {
        TODO("Add error handling");
        ret = plugin_manager_provider->init(plugin_manager_context);
    }

    PluginManagerInterface plugin_manager_interface = {
        .vtable = (const PluginManagerVtable *)plugin_manager_vtable,
        .context = plugin_manager_context,
    };

    plugin_manager_pm_bootstrap(&plugin_manager_interface,
                                plugin_manager_metadata,
                                argc, argv, platform_context,
                                plugin_registry,
                                static_plugin_metadatas,
                                requested_plugins);

    ret = plugin_manager_bootloader_main(&plugin_manager_interface);

    if (plugin_manager_provider->shutdown)
    {
        ret = plugin_manager_provider->shutdown(plugin_manager_context);
    }

    plugin_manager_provider->destroy_context(plugin_manager_context);
    return ret;
}