#pragma once

#include <stdint.h>
#include <stddef.h>

#pragma pack(push, 8)

struct PluginRegistry;
const struct PluginRegistry *get_plugin_registry(void);

struct PluginMetadata;

typedef struct BootloaderPluginMetadatas
{
    const struct PluginMetadata *const *plugin_metadatas;
    const struct PluginMetadata *plugin_manager_metadata;
} BootloaderPluginMetadatas;

const BootloaderPluginMetadatas *get_bootloader_plugin_metadatas(void);

struct RequestedPlugin;

const struct RequestedPlugin *get_bootloader_requested_plugins(void);

#pragma pack(pop)