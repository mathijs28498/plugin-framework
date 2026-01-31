#include "test_plugin.h"

#include <test_api_2.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

TestApi2 *test_api_2;

// TODO: Add api name in front of 
__declspec(dllexport) void get_dependencies(const char *const **dependencies, int32_t *count)
{
    static const char *const plugin_dependencies[] = {
        "test_api_2",
    };

    assert(dependencies != NULL);
    assert(count != NULL);

    *dependencies = plugin_dependencies;
    *count = sizeof(plugin_dependencies) / sizeof(plugin_dependencies[0]);

    printf("%s - %s()\n", __FILE__, __func__);
}

__declspec(dllexport) void set_dependency_test_api_2(void *api)
{
    test_api_2 = (TestApi2 *)api;
    printf("%s - %s()\n", __FILE__, __func__);
}
