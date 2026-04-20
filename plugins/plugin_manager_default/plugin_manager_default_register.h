#pragma once

#include <plugin_sdk/plugin_utils.h>

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

typedef struct PluginScope
{
    enum PluginLifetime lifetime;
    ARRAY_FIELD(struct ScopedPlugin, plugins, MAX_INTERFACE_PER_SCOPE);
} PluginScope;

typedef struct RegisteredPlugin
{
    // When a lifetime is set it cannot be changed, if a different lifetime is requested while the lifetime is not PLUGIN_LIFETIME_UNKNOWN it is an error
    enum PluginLifetime lifetime;
    void *platform_handle;
    const struct PluginMetadata *metadata;
} RegisteredPlugin;

typedef struct PluginManagerContext
{
    TODO("Make sure that the PluginDependencies deps; file is correctly formatted")
    PluginDependencies deps;
    TODO("Maybe DO add the plugin_manager as registered plugin so that other plugins can have it as a dependency for scopes, just look into how to handle shutdown and destroy")
    ARRAY_FIELD(struct RegisteredPlugin, registered_plugins, MAX_REGISTERED_PLUGINS_LEN);
    PluginScope singleton_scope;
} PluginManagerContext;

#pragma pack(pop)