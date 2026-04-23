#pragma once

#include <stdint.h>
#include <stddef.h>

#pragma pack(push, 8)

struct PluginRegistry;
const struct PluginRegistry *get_plugin_registry(void);

struct PluginMetadata;
int32_t get_bootloader_plugin_metadatas(
    const struct PluginRegistry *plugin_registry,
    const struct PluginMetadata *const **out_plugin_metadatas,
    const struct PluginMetadata **out_plugin_manager_metadata);

struct RequestedPlugin;

const struct RequestedPlugin *get_bootloader_requested_plugins(void);

struct PluginFrameworkMemory;
struct PluginFrameworkMemory *get_plugin_framework_memory(void);

#pragma pack(pop)