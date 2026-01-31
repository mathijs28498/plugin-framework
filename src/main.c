#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// #include <test_plugin.h>
#include <Windows.h>

// #include <jsmn.h>
// #include <cJSON.h>
// #include <test_yes.h>
// #include <test_lib.h>

#include <test_api.h>

#include <plugin_loader.h>
#include <plugin_api.h>



int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int32_t ret;

    ret = PLUGIN_API_ADD("test_api", NULL);
    ret = PLUGIN_API_ADD("test_api2", NULL);
    // ret = PLUGIN_API_ADD("test_api3", NULL);
    ret = PLUGIN_API_INIT();

    // PluginApi *plugin_api = get_plugin_api();
    // ret = plugin_api->add("test_api", NULL);
    // ret = plugin_api->init();


    // ret = plugin_loader_init();

    // PluginApi *plugin_api = get_plugin_api();

    // plugin_api->add_search_path("./");

    // plugin_api->get("test", NULL);

    // HMODULE test_plugin = LoadLibrary("TestPlugin.dll");
    // if (!test_plugin)
    // {
    //     printf("Failed to load plugin\n");
    //     return -1;
    // }

    // FARPROC proc = GetProcAddress(test_plugin, "test_func");
    // if (!proc)
    // {
    //     printf("Failed to get proc\n");
    //     return -1;
    // }
    // proc(1300);

    // test_func(1300);
}