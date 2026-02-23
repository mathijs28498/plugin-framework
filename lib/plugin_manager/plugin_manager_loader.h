#pragma once

#include <stdint.h>
#include <stddef.h>

struct LoggerInterface;
struct RequestedPlugin;
struct PluginRegistry;
struct PluginModule;

struct InterfaceInstance;
struct PluginStatic;

int32_t resolve_requested_plugins_registry(
    const struct LoggerInterface *logger,
    struct RequestedPlugin *requested_plugins,
    size_t requested_plugins_len,
    const struct PluginRegistry *plugin_registry,
    struct PluginModule *plugin_modules,
    size_t *plugin_modules_len);
int32_t load_plugin_modules(
    const struct LoggerInterface *logger,
    struct PluginModule *plugin_modules,
    size_t plugin_modules_len,
    struct InterfaceInstance *interface_instances,
    size_t *interface_instances_len);

int32_t resolve_requested_plugins_internal(
    const struct LoggerInterface *logger,
    struct RequestedPlugin *requested_plugins,
    size_t requested_plugins_len,
    const struct PluginStatic *internal_plugins,
    size_t internal_plugins_len,
    // struct PluginStatic *static_plugins_list,
    // size_t *static_plugins_len,
    struct InterfaceInstance *interface_instances,
    size_t *interface_instances_len);

int32_t resolve_plugin_module_dependencies(
    const struct LoggerInterface *logger,
    const struct InterfaceInstance *interface_instances,
    size_t interface_instances_len,
    struct PluginModule *plugin_modules,
    size_t plugin_modules_len,
    struct RequestedPlugin *requested_plugins,
    size_t *requested_plugins_len);

int32_t calculate_plugin_module_initialization_order(
    const struct LoggerInterface *logger,
    const struct PluginModule *plugin_modules,
    size_t plugin_modules_len,
    uint32_t *sorted_plugin_modules_indices);

int32_t initialize_plugins(
    const struct LoggerInterface *logger,
    uint32_t *sorted_plugin_modules_indices,
    struct PluginModule *plugin_modules,
    size_t plugin_modules_len);