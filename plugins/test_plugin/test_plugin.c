#include "test_plugin.h"

#include <test_api_2.h>

#include <stdint.h>
#include <stdio.h>

TestApi2 *test_api_2;

__declspec(dllexport) void get_dependencies__(const char** dependencies, int32_t *count){
    (void) dependencies;
    (void) count;

    printf("%s()\n", __func__);
}

__declspec(dllexport)  void set_dependency_test_api_2(void *api)
{
    test_api_2 = api;
    printf("%s()\n", __func__);
}


void test_func(uint32_t num)
{
    printf("This works!: %d\n", num);
}