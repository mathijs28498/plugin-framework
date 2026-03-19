#include "plugin_manager_bootloader.h"

#include <stdint.h>
#include <stdbool.h>

#include <plugin_sdk_types.h>
#include <plugin_utils.h>
#include <plugin_manager_interface.h>
#include <plugin_manager_pm_interface.h>

// #include "plugin_registry.h"
// #include "plugin_metadata.h"
#include "plugin_manager_bootloader_generated.h"

int32_t plugin_manager_bootloader_main(PluginManagerInterface *plugin_manager);

int32_t plugin_manager_bootloader_bootstrap(int argc, char *argv[], void *platform_context)
{
    int32_t ret;

    const PluginRegistry *plugin_registry = get_plugin_registry();
    const BootloaderPluginMetadatas *bootloader_plugin_metadatas = get_bootloader_plugin_metadatas();
    const BootloaderRequestedPlugins *bootloader_requested_plugins = get_bootloader_requested_plugins();

    const PluginProvider *plugin_manager_provider = bootloader_plugin_metadatas->plugin_manager_metadata->provider;
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
                                argc, argv, platform_context,
                                plugin_registry,
                                bootloader_plugin_metadatas->plugin_metadatas,
                                bootloader_plugin_metadatas->plugin_metadatas_len,
                                bootloader_requested_plugins->requested_plugins,
                                bootloader_requested_plugins->requested_plugins_len);

    ret = plugin_manager_bootloader_main(&plugin_manager_interface);

    if (plugin_manager_provider->shutdown)
    {
        ret = plugin_manager_provider->shutdown(plugin_manager_context);
    }

    plugin_manager_provider->destroy_context(plugin_manager_context);
    return ret;
}
TODO("Implement this")
/*
- Populate plugin_metadata array with generated static methods
- Initialize
- Somehow get the data to the pl
*/

// const PluginRegistry *plugin_registry_get(void);

struct PluginManagerSetupContext;
struct PluginManagerRuntimeContext;

// int32_t __plugin_manager_init(int argc, char **argv, void *platform_context, struct PluginManagerSetupContext **setup_context, struct PluginManagerRuntimeContext **runtime_context)
// {
//     (void)argc;
//     (void)argv;
//     (void)platform_context;
//     (void)setup_context;
//     (void)runtime_context;
//     TODO("Put this in generated code")
//     // const PluginMetadata *plugin_manager_metadata = plugin_manager_default_get_plugin_metadata();

//     const PluginProvider *plugin_manager_provider = plugin_manager_metadata->provider;
//     (void)plugin_manager_provider;
//     return 0;
// }

// int32_t __plugin_manager_get(struct PluginManagerRuntimeContext *runtime_context, const char *interface_name, void **iface)
// {
//     (void)runtime_context;
//     (void)interface_name;
//     (void)iface;
//     return 0;
// }

// int32_t __plugin_manager_shutdown(struct PluginManagerSetupContext *setup_context, struct PluginManagerRuntimeContext *runtime_context, int exit_code)
// {
//     (void)setup_context;
//     (void)runtime_context;
//     (void)exit_code;
//     return 0;
// }