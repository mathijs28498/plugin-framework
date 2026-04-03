#pragma once

#include <plugin_utils.h>

#include "plugin_dependencies.h"

#define MAX_REGISTERED_PLUGINS_LEN 64
#define MAX_INTERFACE_PER_SCOPE 64

struct PluginMetadata;
enum PluginLifetime;

#pragma pack(push, 8)

typedef struct ScopedPluginInterface
{
    void *context;
    const void *vtable;
} ScopedPluginInterface;

typedef struct ScopedPlugin
{
    const char *interface_name;
    ScopedPluginInterface iface;
} ScopedPlugin;

TODO("Improve this iteratively")
typedef struct PluginScope
{
    enum PluginLifetime lifetime;
    ScopedPlugin plugins[MAX_INTERFACE_PER_SCOPE];
    size_t plugins_len;
} PluginScope;

TODO("See if this name is correct")
typedef struct RegisteredPlugin
{
    enum PluginLifetime lifetime;
    const struct PluginMetadata *metadata;
} RegisteredPlugin;

typedef struct PluginManagerContext
{
    TODO("Make sure that the PLUGIN_CONTEXT_DEPENDENCIES file is correctly formatted")
    PLUGIN_CONTEXT_DEPENDENCIES
    struct RegisteredPlugin registered_plugins[MAX_REGISTERED_PLUGINS_LEN];
    size_t registered_plugins_len;
    PluginScope singleton_scope;
} PluginManagerContext;

#pragma pack(pop)