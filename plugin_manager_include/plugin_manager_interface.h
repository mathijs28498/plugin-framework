#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "plugin_utils.h"

struct PluginManagerContext;

#pragma pack(push, 8)


TODO("Figure out how to make iface type not be void **, perhaps make a macro for the getter instead to fix this")
typedef struct PluginManagerVtable
{
    int32_t (*get_singleton)(struct PluginManagerContext *context, const char *interface_name, void **iface);
} PluginManagerVtable;


typedef struct PluginManagerInterface
{
    struct PluginManagerContext *context;
    const PluginManagerVtable *vtable;

} PluginManagerInterface;

#pragma pack(pop)

TODO("Create macro for get_singleton")
TODO("Create other scoped functions")