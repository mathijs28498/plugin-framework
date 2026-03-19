#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <plugin_utils.h>

#include <plugin_sdk_types.h>


typedef struct RequestedPlugin
{
    const char *interface_name;
    const char *plugin_name;
    bool is_explicit;
    bool is_resolved;
} RequestedPlugin;

// typedef struct PluginInterface
// {
//     void *context;
//     void *vtable;
// } PluginInterface;

// typedef struct RegisteredInterface
// {
//     const char *interface_name;
//     const PluginProvider *plugin_provider;
//     const void *plugin_vtable;
//     const bool is_explicit;
// } RegisteredInterface;

// typedef struct InterfaceInstance
// {
//     RegisteredInterface *registered_interface;
//     void *context;
// } InterfaceInstance;

// TODO("Make this into cmake variables")
// #define PLUGIN_REGISTRY_MAX_PLUGIN_LEN 64
// #define PLUGIN_REGISTRY_MAX_PLUGIN_NAME_LEN 64
// #define PLUGIN_REGISTRY_MAX_PLUGIN_PATH_LEN 512
// #define PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN 64
// #define PLUGIN_MANAGER_MAX_PLUGINS_LEN 64

// #define PLUGIN_MANAGER_MAX_DEPENDENCIES 64























// typedef struct PluginProviderDependency
// {
//     const char *interface_name;
//     bool is_resolved;
//     PluginSetDependency_Fn set;
// } PluginProviderDependency;

// struct PluginDefinition;
// typedef struct PluginModule
// {
//     const struct PluginDefinition *plugin_definition;
//     bool is_explicit;
// } PluginModule;

// TODO("Figure out if needs to remember the handle for shutdown")
// typedef struct PluginProvider
// {
//     const char *interface_name;
//     const char *plugin_name;

//     PluginProviderDependency dependencies[PLUGIN_MANAGER_MAX_DEPENDENCIES];
//     uint32_t dependencies_len;

//     PluginGetInterface_Fn get_interface;
//     PluginInit_Fn init;
//     PluginShutdown_Fn shutdown;
//     bool is_initialized;
//     bool is_explicit;
// } PluginProvider;

// struct LoggerInterface;
// struct EnvironmentInterface;

// typedef struct PluginManagerSetupContext
// {
//     struct LoggerInterface *logger;
//     struct EnvironmentInterface *environment;

//     RequestedPlugin requested_plugins[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
//     size_t requested_plugins_len;

//     size_t sorted_plugin_providers_indices[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
//     PluginProvider plugin_providers[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
//     size_t plugin_providers_len;
// } PluginManagerSetupContext;

// typedef struct InterfaceInstance
// {
//     const char *interface_name;
//     PluginManagerBaseInterface *iface;
//     bool is_explicit;
// } InterfaceInstance;

// typedef struct PluginManagerRuntimeContext
// {
//     struct LoggerInterface *logger;
//     InterfaceInstance interface_instances[PLUGIN_MANAGER_MAX_PLUGINS_LEN];
//     size_t interface_instances_len;
// } PluginManagerRuntimeContext;
