#pragma once
#ifndef PLUGIN_API_H
#define PLUGIN_API_H

#include <plugin_manager_common.h>
#include <stdint.h>

TODO("Add static linking functionality")

#ifndef PLUGIN_MANAGER_STATIC_LINKING

struct PluginManagerRuntimeContext;
struct PluginManagerSetupContext;

struct PluginManagerRuntimeContext *get_plugin_manager_runtime_context();

int32_t plugin_manager_init(struct PluginManagerSetupContext **setup_context, int argc, char **argv);
int32_t plugin_manager_add(struct PluginManagerSetupContext *setup_context, const char *api_name, const char *plugin_name);
int32_t plugin_manager_load(struct PluginManagerSetupContext *setup_context, struct PluginManagerRuntimeContext *runtime_context);
int32_t plugin_manager_get(const struct PluginManagerRuntimeContext *runtime_context, const char *api_name, void **api_interface);

#endif // #ifndef STATIC_LINKING

TODO("Add wWinMain here when necessary")
#define PLUGIN_MANAGER_API_MAIN()                                     \
    int plugin_api_main(struct PluginManagerSetupContext *__context); \
    int main(int argc, char *argv[])                                  \
    {                                                                 \
        struct PluginManagerSetupContext *__context;                  \
        (void)plugin_manager_init(&__context, argc, argv);            \
        return plugin_api_main(__context);                                     \
    }                                                                 \
    int plugin_api_main(struct PluginManagerSetupContext *__context)

#ifdef PLUGIN_MANAGER_STATIC_LINKING
#define PLUGIN_MANAGER_API_ADD(api_name, plugin_name)
#else
#define PLUGIN_MANAGER_API_ADD(api_name, plugin_name) (plugin_manager_add(__context, api_name, plugin_name))
#endif // #ifdef PLUGIN_MANAGER_STATIC_LINKING

#ifdef PLUGIN_MANAGER_STATIC_LINKING
#define PLUGIN_MANAGER_API_LOAD()
#else
#define PLUGIN_MANAGER_API_LOAD() (plugin_manager_load(__context, get_plugin_manager_runtime_context()))
#endif // #ifdef PLUGIN_MANAGER_STATIC_LINKING

#ifdef PLUGIN_MANAGER_STATIC_LINKING
#define PLUGIN_MANAGER_API_GET(api_name, out_api_interface)
#else
#define PLUGIN_MANAGER_API_GET(api_name, out_api_interface) (plugin_manager_get(get_plugin_manager_runtime_context(), api_name, out_api_interface))
#endif // #ifdef PLUGIN_MANAGER_STATIC_LINKING

#endif // #ifndef PLUGIN_API_H