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

#include <plugin_manager_api.h>
#include <plugin_manager.h>

PLUGIN_MANAGER_API_MAIN()
{
    int32_t ret;

    ret = PLUGIN_MANAGER_API_ADD("test_api", NULL);

    ret = PLUGIN_MANAGER_API_LOAD();
    return 0;
}