#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "plugin_utils.h"

struct PluginManagerContext;
struct PluginScope;

#pragma pack(push, 8)

typedef struct PluginManagerVtable
{
    int32_t (*get_singleton)(struct PluginManagerContext *context, const char *interface_name, void **out_iface);
    int32_t (*get_scoped)(struct PluginManagerContext *context, struct PluginScope *scope, const char *interface_name, void **out_iface);
} PluginManagerVtable;

typedef struct PluginManagerInterface
{
    struct PluginManagerContext *context;
    const PluginManagerVtable *vtable;

} PluginManagerInterface;

#pragma pack(pop)

#define PLUGIN_MANAGER_GET_SINGLETON(plugin_manager_iface, interface_name, out_iface) \
    (plugin_manager_iface)->vtable->get_singleton((plugin_manager_iface)->context, (interface_name), (void **)(out_iface))

#define PLUGIN_MANAGER_GET_SCOPED(plugin_manager_iface, scope, interface_name, out_iface) \
    (plugin_manager_iface)->vtable->get_scoped((plugin_manager_iface)->context, (scope), (interface_name), (void **)(out_iface))