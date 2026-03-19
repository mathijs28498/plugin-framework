#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct LoggerInterface;
struct RequestedPlugin;
struct PluginProvider;
struct InterfaceInstance;



#if PLUGIN_BUILD_SHARED
struct PluginRegistry;
struct PluginModule;
struct PluginDefinition;

int32_t resolve_requested_plugins(
    const struct LoggerInterface *logger,
    struct RequestedPlugin *requested_plugins,
    size_t requested_plugins_len,
    const struct PluginRegistry *plugin_registry,
    struct PluginProvider *plugin_providers,
    size_t plugin_providers_len,
    struct PluginModule *plugin_modules,
    size_t *plugin_modules_len);

int32_t load_plugin_modules(
    const struct LoggerInterface *logger,
    const struct PluginModule *plugin_modules,
    size_t plugin_modules_len,
    struct PluginProvider *interface_providers,
    size_t *interface_providers_len);

int32_t calculate_plugin_provider_initialization_order(
    const struct LoggerInterface *logger,
    const struct PluginProvider *plugin_providers,
    size_t plugin_providers_len,
    size_t *sorted_plugin_providers_indices);
#endif // #if PLUGIN_BUILD_SHARED

int32_t resolve_plugin_provider_dependencies(
    const struct LoggerInterface *logger,
    bool discover_dependencies,
    struct PluginProvider *plugin_providers,
    size_t plugin_providers_len,
    struct RequestedPlugin *requested_plugins,
    size_t *requested_plugins_len);

int32_t initialize_plugins(
    const struct LoggerInterface *logger,
    const size_t *sorted_plugin_providers_indices,
    struct PluginProvider *plugin_providers,
    size_t plugin_providers_len,
    struct InterfaceInstance *interface_instances,
    size_t *interface_instances_len);