#pragma once
#ifndef PLUGIN_MANAGER_TYPES_H
#define PLUGIN_MANAGER_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// TODO: Make this into cmake variables
#define PLUGIN_REGISTRY_MAX_PLUGIN_LEN 64
#define PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN 64
#define PLUGIN_REGISTRY_MAX_PLUGIN_PATH_LEN 512
#define PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN 64
#define PLUGIN_MANAGER_MAX_PLUGINS_LEN 64

#define PLUGIN_MANAGER_MAX_INTERNAL_PLUGINS_LEN 2
#define PLUGIN_MANAGER_MAX_DEPENDENCIES 64

typedef struct PluginDefinition
{
    char api_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN];
    char plugin_name[PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN];
    char path[PLUGIN_REGISTRY_MAX_PLUGIN_PATH_LEN];
} PluginDefinition;

typedef struct PluginRegistry
{
    uint32_t plugin_definitions_len;
    PluginDefinition plugin_definitions[PLUGIN_REGISTRY_MAX_PLUGIN_LEN];
} PluginRegistry;

typedef struct RequestedPlugin
{
    char api_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN];
    char plugin_name[PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN];
    bool resolved;
} RequestedPlugin;

#ifndef _WINDEF_
struct HINSTANCE__;
typedef struct HINSTANCE__ *HMODULE;
#endif

typedef struct PluginManagerBaseApi
{
    void *context;
} PluginManagerBaseApi;

typedef void (*PluginGetDependencies_Fn)(const char *const **dependencies, uint32_t *len);
typedef void (*PluginSetDependency_Fn)(PluginManagerBaseApi *context, void *api);
typedef PluginManagerBaseApi *(*PluginGetApi_Fn)(void);
typedef int32_t (*PluginInit_Fn)(void *PluginManagerBaseApi);

typedef struct PluginDependency
{
    char *api_name;
    bool resolved;
    PluginSetDependency_Fn set;
} PluginDependency;

typedef struct PluginModule
{
    const PluginDefinition *plugin_definition;
    // HMODULE handle;

    PluginDependency dependencies[PLUGIN_MANAGER_MAX_DEPENDENCIES];
    uint32_t dependencies_len;

    // PluginManagerBaseApi *api;
    PluginGetApi_Fn get_api;
    PluginInit_Fn init;
} PluginModule;

typedef struct PluginStatic
{
    const char api_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN];
    const char plugin_name[PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN];

    // char **dependencies;
    PluginDependency *dependencies;
    uint32_t dependencies_len;
    PluginManagerBaseApi *api;
} PluginStatic;

struct LoggerApi;

typedef struct PluginManagerSetupContext
{
    struct LoggerApi *logger_api;

    size_t internal_plugins_len;
    struct PluginStatic internal_plugins[PLUGIN_MANAGER_MAX_INTERNAL_PLUGINS_LEN];

    size_t requested_plugins_len;
    RequestedPlugin requested_plugins[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
} PluginManagerSetupContext;

typedef struct ApiInstance
{
    char api_name[PLUGIN_REGISTRY_MAX_PLUGIN_API_NAME_LEN];
    PluginManagerBaseApi *api;
} ApiInstance;

typedef struct PluginManagerRuntimeContext
{
    size_t api_instances_len;
    ApiInstance api_instances[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
} PluginManagerRuntimeContext;

#endif // #ifndef PLUGIN_MANAGER_TYPES_H