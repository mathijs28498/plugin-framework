#include <plugin_utils.h>
#include <stdint.h>
#include <plugin_manager_interface.h>

#include "plugin_manager_bootloader.h"

TODO("Make sure CoInitialize and Uninitialize are both properly done for both main and wWinMain when necessary")
TODO("Allow for wWinMain and winMain based on if _UNICODE is defined")

#if WINDOWS_GUI
#include <Windows.h>
#include <stdlib.h>
#include <malloc.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;

    HRESULT hr = {0};
    TODO("Figure out where to do this, if this is part of plugin_manager interface, plugin_manager_bootloader, or here in the main/ in the other main function")
    hr = CoInitialize(NULL);

    TODO("Make sure the windows places that use it know how the struct looks like, maybe define this inside of environment_interface.h");
    static struct
    {
        void *hInstance;
        int nCmdShow;
    } platform_context;
    platform_context.hInstance = hInstance;
    platform_context.nCmdShow = nCmdShow;

    char **argv = (char **)_malloca(__argc * sizeof(char *));

    for (int i = 0; i < __argc; i++)
    {
        int arg_size = WideCharToMultiByte(CP_UTF8, 0, __wargv[i], -1, NULL, 0, NULL, NULL);
        argv[i] = (char *)_malloca(arg_size);
        WideCharToMultiByte(CP_UTF8, 0, __wargv[i], -1, argv[i], arg_size, NULL, NULL);
    }

    // ret = __plugin_manager_init(__argc, __argv, &platform_context, &__setup_context, &__runtime_context);
    // if (ret < 0)
    // {
    //     (void)__plugin_manager_shutdown(__setup_context, __runtime_context, ret);
    //     return ret;
    // }
    int32_t ret = plugin_manager_bootloader_bootstrap(__argc, argv, &platform_context);

    // ret = plugin_manager_main(__runtime_context);
    // (void)__plugin_manager_shutdown(__setup_context, __runtime_context, ret);
    CoUninitialize();

    // HRESULT hr = {0};
    // hr = CoInitialize(NULL);

    // struct PluginManagerSetupContext *__setup_context;
    // struct PluginManagerRuntimeContext *__runtime_context;
    // static struct
    // {
    //     void *hInstance;
    //     int nCmdShow;
    // } platform_context;
    // platform_context.hInstance = hInstance;
    // platform_context.nCmdShow = nCmdShow;

    // int ret;
    // ret = __plugin_manager_init(__argc, __argv, &platform_context, &__setup_context, &__runtime_context);
    // if (ret < 0)
    // {
    //     (void)__plugin_manager_shutdown(__setup_context, __runtime_context, ret);
    //     return ret;
    // }

    // ret = plugin_manager_main(__runtime_context);
    // (void)__plugin_manager_shutdown(__setup_context, __runtime_context, ret);
    // CoUninitialize();
    return (int)ret;
}
#else  // #if WINDOWS_GUI
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int32_t ret;

    ret = plugin_manager_bootloader_main();
    // struct PluginManagerSetupContext *__setup_context;
    // struct PluginManagerRuntimeContext *__runtime_context;
    // int ret;
    // ret = __plugin_manager_init(argc, argv, NULL, &__setup_context, &__runtime_context);
    // if (ret < 0)
    // {
    //     (void)__plugin_manager_shutdown(__setup_context, __runtime_context, ret);
    //     return ret;
    // }

    // ret = plugin_manager_main(__runtime_context);
    // (void)__plugin_manager_shutdown(__setup_context, __runtime_context, ret);
    // return ret;
    return (int)ret;
}
#endif // #if WINDOWS_GUI