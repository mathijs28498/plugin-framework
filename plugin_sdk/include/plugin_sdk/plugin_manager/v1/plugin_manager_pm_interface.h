#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../../plugin_utils.h"
#include "plugin_manager_interface.h"

#pragma pack(push, 8)

typedef struct PluginContextSlabPool
{
    uint8_t *pool;
    const uint64_t pool_size;
    const uint32_t max_plugin_amount;
    uint64_t occupied_bitmap;

} PluginContextSlabPool;

typedef struct PluginMemoryPool
{
    uint8_t *pool;
    const uint64_t pool_size;
} PluginMemoryPool;

typedef struct PluginFrameworkMemory
{
    PluginContextSlabPool context_slab_pool;
    PluginMemoryPool memory_pool;
} PluginFrameworkMemory;

typedef struct PluginDependencyDefinition
{
    const char *interface_name;
    const bool is_deferred;
} PluginDependencyDefinition;

typedef struct PluginDefinition
{
    const char *target_name;
    const char *interface_name;
    const char *plugin_name;
    const PluginDependencyDefinition *dependencies;
    const size_t dependencies_len;
    const bool has_init;
    const bool has_shutdown;
    const char *module_path;

} PluginDefinition;

typedef struct InterfaceDefinition
{
    const char *interface_name;
    const char *default_plugin;
    const PluginDefinition *plugin_definitions;
    size_t plugin_definitions_len;
} InterfaceDefinition;

TODO("Figure out how to handle capacity here")
typedef struct PluginRegistry
{
    const InterfaceDefinition *interface_definitions;
    size_t interface_definitions_len;
} PluginRegistry;

struct PluginMetadata;
enum PluginLifetime;

typedef struct RequestedPlugin
{
    const char *interface_name;
    const char *plugin_name;
    enum PluginLifetime lifetime;
} RequestedPlugin;

typedef struct PluginManagerPMVtable
{
    PluginManagerVtable public_vtable;

    int32_t (*bootstrap)(struct PluginManagerContext *context,
                         const struct PluginMetadata *plugin_manager_metadata,
                         PluginFrameworkMemory *plugin_framework_memory,
                         int argc, char *argv[], void *platform_context,
                         const PluginRegistry *plugin_registry,
                         const struct PluginMetadata *const *static_plugin_metadatas,
                         const RequestedPlugin *requested_plugins_explicit);
} PluginManagerPMVtable;

#pragma pack(pop)

static inline int32_t plugin_manager_pm_bootstrap(
    PluginManagerInterface *iface,
    const struct PluginMetadata *plugin_manager_metadata,
    PluginFrameworkMemory *plugin_framework_memory,
    int argc, char **argv, void *platform_context,
    const PluginRegistry *plugin_registry,
    const struct PluginMetadata *const *static_plugin_metadatas,
    const RequestedPlugin *requested_plugins_explicit)
{
    return ((PluginManagerPMVtable *)iface->vtable)
        ->bootstrap(
            iface->context,
            plugin_manager_metadata,
            plugin_framework_memory,
            argc, argv, platform_context,
            plugin_registry,
            static_plugin_metadatas,
            requested_plugins_explicit);
}