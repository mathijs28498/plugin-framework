#pragma once

#include <stdint.h>
#include <stdbool.h>

struct PluginMetadata;
enum PluginLifetime;

struct PluginManagerContext;
struct PluginScope;
struct LoggerInterface;
struct RegisteredPlugin;
struct ScopedPluginInterface;
struct PluginContextSlabPool;

bool is_lifetime_supported(const struct PluginMetadata *plugin_metadata, enum PluginLifetime lifetime);

int32_t plugin_manager_default_get_singleton(struct PluginManagerContext *context, const char *interface_name, void **out_iface);
int32_t plugin_manager_default_get_scoped(struct PluginManagerContext *context, struct PluginScope *scope, const char *interface_name, void **out_iface);

int32_t add_plugins_to_scope(const struct LoggerInterface *logger,
                             struct PluginContextSlabPool *context_slab_pool,
                             struct PluginScope *singleton_scope,
                             struct RegisteredPlugin *registered_plugins,
                             const char **interface_names_to_add,
                             struct PluginScope *scope);

int32_t add_plugin_to_scope(const struct LoggerInterface *logger,
                            struct PluginContextSlabPool *context_slab_pool,
                            struct PluginScope *singleton_scope,
                            struct RegisteredPlugin *registered_plugins,
                            const char *interface_name_to_add,
                            struct PluginScope *scope,
                            struct ScopedPluginInterface **out_iface);

int32_t plugin_manager_default_shutdown_scope(struct PluginContextSlabPool *context_slab_pool,
                                              struct RegisteredPlugin *registered_plugins,
                                              struct PluginScope *scope);