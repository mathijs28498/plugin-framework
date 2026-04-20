#include "plugin_manager_bootloader.h"
#include <plugin_sdk/plugin_utils.h>

#include <stdint.h>
#include <stdbool.h>
TODO("Make sure this is in a separate file")
#include <Windows.h>
#include <assert.h>
#include <stdio.h>

#include <plugin_sdk/plugin_sdk_types.h>
#include <plugin_sdk/plugin_manager_interface.h>
#include <plugin_sdk/plugin_manager_pm_interface.h>

#include "plugin_manager_bootloader_generated.h"

#define PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN 256

int32_t plugin_manager_bootloader_main(PluginManagerInterface *plugin_manager);

int32_t plugin_manager_bootloader_resolve_get_metadata_fn_dynamic(const char *module_path, const char *target_name, PluginGetMetadata_Fn *out_get_metadata_fn)
{
    assert(module_path != NULL);
    assert(target_name != NULL);
    assert(out_get_metadata_fn != NULL);

    int32_t ret;

    WCHAR wide_path[MAX_PATH];

    ret = (int32_t)MultiByteToWideChar(
        CP_UTF8,     // Assume the input is UTF-8 (use CP_ACP for ANSI)
        0,           // No special flags
        module_path, // The source string
        -1,          // -1 means the string is null-terminated
        wide_path,   // The destination buffer
        MAX_PATH     // The size of the destination buffer
    );

    if (ret == 0)
    {
        // DWORD err = GetLastError();
        return ret;
    }

    HMODULE handle = LoadLibrary(wide_path);
    if (!handle)
    {
        TODO("Add error log")
        return -1;
    }

    char function_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
    snprintf(function_name, sizeof(function_name), "%s_get_plugin_metadata", target_name);

    FARPROC proc_address = GetProcAddress(handle, function_name);

    if (!proc_address)
    {
        TODO("Add error log");
        FreeLibrary(handle);
        return -2;
    }

    *out_get_metadata_fn = (PluginGetMetadata_Fn)proc_address;

    return 0;
}

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

int32_t plugin_manager_bootloader_bootstrap(int argc, char **argv, void *platform_context)
{
    int32_t ret;

    const PluginRegistry *plugin_registry = get_plugin_registry();
    const RequestedPlugin *requested_plugins = get_bootloader_requested_plugins();

#if PLUGIN_BUILD_SHARED
    const PluginDefinition *plugin_manager_plugin_definition;
    ret = get_plugin_manager_definition(plugin_registry, &plugin_manager_plugin_definition);
    if (ret < 0)
    {
        TODO("Add error log");
        return -1;
    }

    PluginGetMetadata_Fn get_metadata_fn = NULL;
    ret = plugin_manager_bootloader_resolve_get_metadata_fn_dynamic(
        plugin_manager_plugin_definition->module_path, plugin_manager_plugin_definition->target_name, &get_metadata_fn);
    if (ret < 0)
    {
        TODO("Add error log");
        return ret;
    }

    const PluginMetadata *plugin_manager_metadata = get_metadata_fn();
    CREATE_ARRAY(const struct PluginMetadata *const, static_plugin_metadatas, 1);
#else  // #if !PLUGIN_BUILD_SHARED
    const BootloaderPluginMetadatas *bootloader_plugin_metadatas = get_bootloader_plugin_metadatas();
    const PluginMetadata *plugin_manager_metadata = bootloader_plugin_metadatas->plugin_manager_metadata;
    const struct PluginMetadata *const *static_plugin_metadatas = bootloader_plugin_metadatas->plugin_metadatas;
#endif // #if !PLUGIN_BUILD_SHARED

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