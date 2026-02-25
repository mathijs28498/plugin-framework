#pragma once

#include <stdint.h>
#include <stddef.h>

struct LoggerInterface;
struct RequestedPlugin;
struct PluginModuleRegistry;
struct PluginModule;

struct PluginProvider;
struct InterfaceInstance;
struct PluginStatic;

int32_t resolve_requested_plugins_registry(
    const struct LoggerInterface *logger,
    struct RequestedPlugin *requested_plugins,
    size_t requested_plugins_len,
    const struct PluginModuleRegistry *plugin_registry,
    struct PluginModule *plugin_modules,
    size_t *plugin_modules_len);

int32_t load_plugin_modules(
    const struct LoggerInterface *logger,
    const struct PluginModule *plugin_modules,
    size_t plugin_modules_len,
    struct PluginProvider *interface_providers,
    size_t *interface_providers_len);

int32_t resolve_requested_plugins_internal(
    const struct LoggerInterface *logger,
    struct RequestedPlugin *requested_plugins,
    size_t requested_plugins_len,
    const struct PluginProvider *internal_plugins,
    size_t internal_plugins_len,
    struct PluginProvider *interface_providers,
    size_t *interface_providers_len);

int32_t resolve_plugin_provider_dependencies(
    const struct LoggerInterface *logger,
    struct PluginProvider *plugin_providers,
    size_t plugin_providers_len,
    struct RequestedPlugin *requested_plugins,
    size_t *requested_plugins_len);

int32_t calculate_plugin_provider_initialization_order(
    const struct LoggerInterface *logger,
    const struct PluginProvider *plugin_providers,
    size_t plugin_providers_len,
    size_t *sorted_plugin_providers_indices);

int32_t initialize_plugins(
    const struct LoggerInterface *logger,
    const size_t *sorted_plugin_providers_indices,
    const struct PluginProvider *plugin_providers,
    size_t plugin_providers_len,
    struct InterfaceInstance *interface_instances,
    size_t *interface_instances_len);