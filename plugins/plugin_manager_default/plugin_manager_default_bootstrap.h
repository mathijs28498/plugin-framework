#pragma once

#include <stdint.h>

#include <plugin_sdk/plugin_utils.h>

TODO("Figure out where these belong")
#define PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN 256
#define PLUGIN_MANAGER_RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH 256

struct PluginManagerContext;
struct PluginRegistry;
struct PluginMetadata;
struct RequestedPlugin;
struct PluginFrameworkMemory;

int32_t plugin_manager_default_bootstrap(
    struct PluginManagerContext *context,
    const struct PluginMetadata *plugin_manager_metadata,
    struct PluginFrameworkMemory *plugin_framework_memory,
    int argc, char **argv, void *platform_context,
    const struct PluginRegistry *plugin_registry,
    const struct PluginMetadata *const *static_plugin_metadatas,
    const struct RequestedPlugin *requested_plugins_explicit);