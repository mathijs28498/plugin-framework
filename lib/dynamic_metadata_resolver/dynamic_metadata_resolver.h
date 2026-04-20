#pragma once

#include <stdint.h>

#define PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN 256

struct PluginMetadata;

typedef const struct PluginMetadata *(*PluginGetMetadata_Fn)(void);

int32_t resolve_get_metadata_fn_dynamic(const char *module_path, const char *target_name, PluginGetMetadata_Fn *out_get_metadata_fn);