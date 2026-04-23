#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../../plugin_utils.h"

#pragma pack(push, 8)

typedef int32_t (*PluginInjectDependency_Fn)(void *context, const char *interface_name, void *iface);
typedef int32_t (*PluginInit_Fn)(void *context);
typedef int32_t (*PluginShutdown_Fn)(void *context);

typedef struct PluginProvider
{
    const void *vtable;
    PluginInjectDependency_Fn inject_dependency;
    PluginInit_Fn init;
    PluginShutdown_Fn shutdown;
    uint64_t context_size;
} PluginProvider;

typedef struct PluginDependency
{
    const char *interface_name;
} PluginDependency;

typedef enum PluginLifetime
{
    PLUGIN_LIFETIME_UNKNOWN = 0,
    PLUGIN_LIFETIME_SINGLETON = 1,
    PLUGIN_LIFETIME_SCOPED = 2,
    PLUGIN_LIFETIME_TRANSIENT = 3,
} PluginLifetime;

TODO("Add optional dependencies")
typedef struct PluginMetadata
{
    const char *interface_name;
    const char *plugin_name;

    TODO("Check how to add capacity and stuff here")
    const PluginDependency *dependencies;
    size_t dependencies_len;

    TODO("Check how to add capacity and stuff here")
    const PluginLifetime *supported_lifetimes;
    size_t supported_lifetimes_len;
    const PluginLifetime preferred_lifetime;

    const PluginProvider *provider;
} PluginMetadata;

typedef const PluginMetadata *(*PluginGetMetadata_Fn)(void);


#pragma pack(pop)