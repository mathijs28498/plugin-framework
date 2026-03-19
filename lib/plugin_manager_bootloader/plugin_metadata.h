#pragma once

#include <stdint.h>
#include <stddef.h>

struct PluginMetadata;

int32_t get_plugin_metadatas(const struct PluginMetadata ***out_plugin_metadatas, size_t *out_plugin_metadatas_len, const struct PluginMetadata **out_plugin_manager_metadata);
