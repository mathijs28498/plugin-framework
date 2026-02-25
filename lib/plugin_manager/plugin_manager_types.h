#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <plugin_utils.h>

TODO("Make this into cmake variables")
#define PLUGIN_REGISTRY_MAX_PLUGIN_LEN 64
#define PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN 64
#define PLUGIN_REGISTRY_MAX_PLUGIN_PATH_LEN 512
#define PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN 64
#define PLUGIN_MANAGER_MAX_PLUGINS_LEN 64

#define PLUGIN_MANAGER_MAX_INTERNAL_PLUGINS_LEN 2
#define PLUGIN_MANAGER_MAX_DEPENDENCIES 64

typedef struct PluginModuleDefinition
{
    char interface_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
    char plugin_name[PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN];
    char path[PLUGIN_REGISTRY_MAX_PLUGIN_PATH_LEN];
} PluginModuleDefinition;

typedef struct PluginModuleRegistry
{
    PluginModuleDefinition plugin_definitions[PLUGIN_REGISTRY_MAX_PLUGIN_LEN];
    size_t plugin_definitions_len;
} PluginModuleRegistry;

typedef struct RequestedPlugin
{
    char interface_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
    char plugin_name[PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN];
    bool is_explicit;
    bool is_resolved;
} RequestedPlugin;

typedef struct PluginManagerBaseInterface
{
    void *context;
} PluginManagerBaseInterface;

typedef void (*PluginGetDependencies_Fn)(const char *const **dependencies, uint32_t *len);
typedef void (*PluginSetDependency_Fn)(void *context, void *iface);
typedef PluginManagerBaseInterface *(*PluginGetInterface_Fn)(void);
typedef int32_t (*PluginInit_Fn)(void *context);
typedef int32_t (*PluginShutdown_Fn)(void *context);

typedef struct PluginDependency
{
    char interface_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
    bool is_resolved;
    PluginSetDependency_Fn set;
} PluginDependency;

TODO("Check if this is_explicit can be removed by creating the interface_instance at time of resolving, not of loading")
typedef struct PluginModule
{
    const PluginModuleDefinition *definition;
    bool is_explicit;
} PluginModule;

TODO("Figure out if needs to remember the handle for shutdown")
typedef struct PluginProvider
{
    char interface_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
    char plugin_name[PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN];

    PluginDependency dependencies[PLUGIN_MANAGER_MAX_DEPENDENCIES];
    uint32_t dependencies_len;

    PluginGetInterface_Fn get_interface;
    PluginInit_Fn init;
    PluginShutdown_Fn shutdown;
    bool is_initialized;
    bool is_explicit;
} PluginProvider;

struct LoggerInterface;

typedef struct PluginManagerSetupContext
{
    struct LoggerInterface *logger;

    PluginProvider internal_plugins[PLUGIN_MANAGER_MAX_INTERNAL_PLUGINS_LEN];
    size_t internal_plugins_len;

    RequestedPlugin requested_plugins[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
    size_t requested_plugins_len;

    size_t sorted_plugin_providers_indices[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
    PluginProvider plugin_providers[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
    size_t plugin_providers_len;
} PluginManagerSetupContext;

typedef struct InterfaceInstance
{
    char interface_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
    PluginManagerBaseInterface *iface;
    bool is_explicit;
} InterfaceInstance;

typedef struct PluginManagerRuntimeContext
{
    struct LoggerInterface *logger;
    InterfaceInstance interface_instances[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
    size_t interface_instances_len;
} PluginManagerRuntimeContext;
