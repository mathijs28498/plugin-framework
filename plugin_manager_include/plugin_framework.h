#pragma once

#include <plugin_utils.h>
#include <stdint.h>

TODO("Add static linking functionality")

#ifndef PLUGIN_MANAGER_STATIC_LINKING

struct PluginManagerRuntimeContext;
struct PluginManagerSetupContext;

int32_t __plugin_manager_init(int argc, char **argv, void *platform_context, struct PluginManagerSetupContext **context, struct PluginManagerRuntimeContext **runtime_context);
int32_t __plugin_manager_add(struct PluginManagerSetupContext *setup_context, const char *interface_name, const char *plugin_name);
int32_t __plugin_manager_load(struct PluginManagerSetupContext *setup_context, struct PluginManagerRuntimeContext *runtime_context);
int32_t __plugin_manager_get(struct PluginManagerRuntimeContext *runtime_context, const char *interface_name, void **iface);
int32_t __plugin_manager_shutdown(struct PluginManagerSetupContext *setup_context, struct PluginManagerRuntimeContext *runtime_context, int exit_code);

#endif // #ifndef PLUGIN_MANAGER_STATIC_LINKING

#define __PLUGIN_FRAMEWORK_ADD(interface_name, plugin_name) \
    (void)__plugin_manager_add(__setup_context, interface_name, plugin_name);

TODO("Add instance of runtime context and pass it to the plugin_manager_main, rather than getting it every time")
#if WINDOWS_GUI
#include <Windows.h>
#define __PLUGIN_MANAGER_ENTRY_IMPL()                                                                         \
    int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,                                         \
                        LPWSTR lpCmdLine, int nCmdShow)                                                       \
    {                                                                                                         \
        (void)hPrevInstance;                                                                                  \
        (void)lpCmdLine;                                                                                      \
                                                                                                              \
        HRESULT hr = {0};                                                                                     \
        hr = CoInitialize(NULL);                                                                              \
                                                                                                              \
        struct PluginManagerSetupContext *__setup_context;                                                    \
        struct PluginManagerRuntimeContext *__runtime_context;                                                \
        static struct                                                                                         \
        {                                                                                                     \
            void *hInstance;                                                                                  \
            int nCmdShow;                                                                                     \
        } platform_context;                                                                                   \
        platform_context.hInstance = hInstance;                                                               \
        platform_context.nCmdShow = nCmdShow;                                                                 \
                                                                                                              \
        (void)__plugin_manager_init(__argc, __argv, &platform_context, &__setup_context, &__runtime_context); \
        PLUGIN_FRAMEWORK_PLUGINS_LIST(__PLUGIN_FRAMEWORK_ADD);                                                \
        __plugin_manager_load(__setup_context, __runtime_context);                                            \
        int ret = plugin_manager_main(__runtime_context);                                                     \
        (void)__plugin_manager_shutdown(__setup_context, __runtime_context, ret);                             \
        CoUninitialize();                                                                                     \
        return ret;                                                                                           \
    }
#else // #if WINDOWS_GUI
#define __PLUGIN_MANAGER_ENTRY_IMPL()                                                        \
    int main(int argc, char *argv[])                                                         \
    {                                                                                        \
        struct PluginManagerSetupContext *__setup_context;                                   \
        struct PluginManagerRuntimeContext *__runtime_context;                               \
        (void)__plugin_manager_init(argc, argv, NULL, &__setup_context, &__runtime_context); \
        PLUGIN_FRAMEWORK_PLUGINS_LIST(__PLUGIN_FRAMEWORK_ADD);                               \
        __plugin_manager_load(__setup_context, __runtime_context);                           \
        int ret = plugin_manager_main(__runtime_context);                                    \
        (void)__plugin_manager_shutdown(__setup_context, __runtime_context, ret);            \
        return ret;
}
#endif // #if WINDOWS_GUI

#define PLUGIN_FRAMEWORK_MAIN()                                                     \
    int plugin_manager_main(struct PluginManagerRuntimeContext *__runtime_context); \
    __PLUGIN_MANAGER_ENTRY_IMPL()                                                   \
    int plugin_manager_main(struct PluginManagerRuntimeContext *__runtime_context)

// Maybe change this to work with a function __get_runtime_context() so the _GET can be called anywhere?
#define PLUGIN_FRAMEWORK_GET(interface_name, iface) (__plugin_manager_get(__runtime_context, interface_name, (void **)iface))
