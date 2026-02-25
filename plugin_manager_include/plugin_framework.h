#pragma once

#include <plugin_utils.h>
#include <stdint.h>

TODO("Add static linking functionality")

#ifndef PLUGIN_MANAGER_STATIC_LINKING

struct PluginManagerRuntimeContext;
struct PluginManagerSetupContext;

struct PluginManagerRuntimeContext *__get_plugin_manager_runtime_context();

int32_t __plugin_manager_init(struct PluginManagerSetupContext **context, int argc, char **argv, void *platform_context);
int32_t __plugin_manager_add(struct PluginManagerSetupContext *setup_context, const char *interface_name, const char *plugin_name);
int32_t __plugin_manager_load(struct PluginManagerSetupContext *setup_context, struct PluginManagerRuntimeContext *runtime_context);
int32_t __plugin_manager_get(struct PluginManagerRuntimeContext *runtime_context, const char *interface_name, void **iface);
int32_t __plugin_manager_shutdown(struct PluginManagerSetupContext *setup_context, struct PluginManagerRuntimeContext *runtime_context, int exit_code);

#endif // #ifndef PLUGIN_MANAGER_STATIC_LINKING

TODO("Add instance of runtime context and pass it to the plugin_manager_main, rather than getting it every time")
#if WINDOWS_GUI
#include <Windows.h>
#define __PLUGIN_MANAGER_ENTRY_IMPL()                                                       \
    int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,                       \
                        LPWSTR lpCmdLine, int nCmdShow)                                     \
    {                                                                                       \
        (void)hPrevInstance;                                                                \
        (void)lpCmdLine;                                                                    \
                                                                                            \
        HRESULT hr = {0};                                                                   \
        hr = CoInitialize(NULL);                                                            \
                                                                                            \
        struct PluginManagerSetupContext *__context;                                        \
        static struct                                                                       \
        {                                                                                   \
            void *hInstance;                                                                \
            int nCmdShow;                                                                   \
        } platform_context;                                                                 \
        platform_context.hInstance = hInstance;                                             \
        platform_context.nCmdShow = nCmdShow;                                               \
                                                                                            \
        (void)__plugin_manager_init(&__context, __argc, __argv, &platform_context);         \
        int ret = plugin_manager_main(__context);                                           \
        (void)__plugin_manager_shutdown(__context, __get_plugin_manager_runtime_context(), ret); \
        CoUninitialize();                                                                   \
        return ret;                                                                         \
    }
#else // #if WINDOWS_GUI
#define __PLUGIN_MANAGER_ENTRY_IMPL()                              \
    int main(int argc, char *argv[])                               \
    {                                                              \
        struct PluginManagerSetupContext *__context;               \
        (void)__plugin_manager_init(&__context, argc, argv, NULL); \
        return plugin_manager_main(__context);                     \
    }
#endif // #if WINDOWS_GUI

#define PLUGIN_FRAMEWORK_MAIN()                                           \
    int plugin_manager_main(struct PluginManagerSetupContext *__context); \
    __PLUGIN_MANAGER_ENTRY_IMPL()                                         \
    int plugin_manager_main(struct PluginManagerSetupContext *__context)

#ifdef PLUGIN_MANAGER_STATIC_LINKING
#define PLUGIN_FRAMEWORK_ADD(interface_name, plugin_name)
#else
#define PLUGIN_FRAMEWORK_ADD(interface_name, plugin_name) (__plugin_manager_add(__context, interface_name, plugin_name))
#endif // #ifdef PLUGIN_MANAGER_STATIC_LINKING

#ifdef PLUGIN_MANAGER_STATIC_LINKING
#define PLUGIN_FRAMEWORK_LOAD()
#else
#define PLUGIN_FRAMEWORK_LOAD() (__plugin_manager_load(__context, __get_plugin_manager_runtime_context()))
#endif // #ifdef PLUGIN_MANAGER_STATIC_LINKING

#ifdef PLUGIN_MANAGER_STATIC_LINKING
#define PLUGIN_FRAMEWORK_GET(interface_name, out_interface)
#else
#define PLUGIN_FRAMEWORK_GET(interface_name, iface) (__plugin_manager_get(__get_plugin_manager_runtime_context(), interface_name, (void **)iface))
#endif // #ifdef PLUGIN_MANAGER_STATIC_LINKING
