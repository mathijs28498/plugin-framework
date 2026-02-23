#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <plugin_manager_common.h>

TODO("Make this into cmake variables")
#define PLUGIN_REGISTRY_MAX_PLUGIN_LEN 64
#define PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN 64
#define PLUGIN_REGISTRY_MAX_PLUGIN_PATH_LEN 512
#define PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN 64
#define PLUGIN_MANAGER_MAX_PLUGINS_LEN 64

#define PLUGIN_MANAGER_MAX_INTERNAL_PLUGINS_LEN 2
#define PLUGIN_MANAGER_MAX_DEPENDENCIES 64

typedef struct PluginDefinition
{
    char interface_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
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
    char interface_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
    char plugin_name[PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN];
    bool is_explicit;
    bool resolved;
} RequestedPlugin;

#ifndef _WINDEF_
struct HINSTANCE__;
typedef struct HINSTANCE__ *HMODULE;
#endif

typedef struct PluginManagerBaseInterface
{
    void *context;
} PluginManagerBaseInterface;

typedef void (*PluginGetDependencies_Fn)(const char *const **dependencies, uint32_t *len);
typedef void (*PluginSetDependency_Fn)(PluginManagerBaseInterface *context, void *iface);
typedef PluginManagerBaseInterface *(*PluginGetInterface_Fn)(void);
typedef int32_t (*PluginInit_Fn)(void *PluginManagerBaseInterface);

typedef struct PluginDependency
{
    char *interface_name;
    bool resolved;
    PluginSetDependency_Fn set;
} PluginDependency;


TODO("Check if this is_explicit can be removed by creating the interface_instance at time of resolving, not of loading")
typedef struct PluginModule
{
    const PluginDefinition *plugin_definition;
    bool is_explicit;

    PluginDependency dependencies[PLUGIN_MANAGER_MAX_DEPENDENCIES];
    uint32_t dependencies_len;

    PluginGetInterface_Fn get_interface;
    PluginInit_Fn init;
} PluginModule;

typedef struct PluginStatic
{
    const char interface_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
    const char plugin_name[PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN];

    // char **dependencies;
    PluginDependency *dependencies;
    uint32_t dependencies_len;
    PluginManagerBaseInterface *iface;
} PluginStatic;

struct LoggerInterface;

typedef struct PluginManagerSetupContext
{
    struct LoggerInterface *logger;

    size_t internal_plugins_len;
    struct PluginStatic internal_plugins[PLUGIN_MANAGER_MAX_INTERNAL_PLUGINS_LEN];

    size_t requested_plugins_len;
    RequestedPlugin requested_plugins[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
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
    size_t interface_instances_len;
    InterfaceInstance interface_instances[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
} PluginManagerRuntimeContext;
