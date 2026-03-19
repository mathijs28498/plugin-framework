#pragma once

#include <stdint.h>
#include <stddef.h>

#include "plugin_utils.h"

#pragma pack(push, 8)

typedef void *(*PluginCreateContext_Fn)(void);
typedef int32_t (*PluginInjectDependency_Fn)(void *context, const char *interface_name, void *iface);
typedef int32_t (*PluginInit_Fn)(void *context);
typedef int32_t (*PluginShutdown_Fn)(void *context);
typedef int32_t (*PluginDestroyContext_Fn)(void *context);

typedef struct PluginProvider
{
    const void *vtable;
    PluginCreateContext_Fn create_context;
    PluginInjectDependency_Fn inject_dependency;
    PluginInit_Fn init;
    PluginShutdown_Fn shutdown;
    PluginDestroyContext_Fn destroy_context;
} PluginProvider;

typedef struct PluginDependency
{
    const char *interface_name;
} PluginDependency;

typedef enum PluginLifetime
{
    PLUGIN_LIFETIME_UNKNOWN = -1,
    PLUGIN_LIFETIME_SINGLETON = 0,
    PLUGIN_LIFETIME_SCOPED = 1,
    PLUGIN_LIFETIME_TRANSIENT = 2,
} PluginLifetime;

TODO("Add optional dependencies")
typedef struct PluginMetadata
{
    const char *interface_name;
    const char *plugin_name;

    TODO("Check if this needs dependencies, as PluginDefintion already has those")
    const PluginDependency *dependencies;
    size_t dependencies_len;

    TODO("Check if this needs to be here or needs to PluginDefinition")
    const PluginLifetime *supported_lifetimes;
    size_t supported_lifetimes_len;
    const PluginLifetime preferred_lifetime;

    const PluginProvider *provider;
} PluginMetadata;

typedef const PluginMetadata *(*PluginGetMetadata_Fn)(void);


#pragma pack(pop)