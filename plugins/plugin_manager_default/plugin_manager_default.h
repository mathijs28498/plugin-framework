#pragma once

#include <stdint.h>

struct PluginManagerContext;
struct PluginRegistry;
struct PluginMetadata;
struct RequestedPlugin;

int32_t plugin_manager_default_bootstrap(
    struct PluginManagerContext *context,
    int argc, char **argv, void *platform_context,
    const struct PluginRegistry *plugin_registry,
    const struct PluginMetadata **static_plugin_metadatas, size_t static_plugin_metadatas_len,
    const struct RequestedPlugin *explicitly_requested_plugins, size_t explicitly_requested_plugins_len);

int32_t plugin_manager_default_get_singleton(struct PluginManagerContext *context, const char *interface_name, void **iface);