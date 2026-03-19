#pragma once

#include <stdint.h>
#include <stddef.h>

#pragma pack(push, 8)

struct PluginRegistry;
const struct PluginRegistry *get_plugin_registry(void);

struct PluginMetadata;

typedef struct BootloaderPluginMetadatas
{
    const struct PluginMetadata **plugin_metadatas;
    const size_t plugin_metadatas_len;
    const struct PluginMetadata *plugin_manager_metadata;
} BootloaderPluginMetadatas;

const BootloaderPluginMetadatas *get_bootloader_plugin_metadatas(void);

struct RequestedPlugin;

typedef struct BootloaderRequestedPlugins
{
    const struct RequestedPlugin *requested_plugins;
    const size_t requested_plugins_len;
} BootloaderRequestedPlugins;

const BootloaderRequestedPlugins *get_bootloader_requested_plugins(void);

#pragma pack(pop)