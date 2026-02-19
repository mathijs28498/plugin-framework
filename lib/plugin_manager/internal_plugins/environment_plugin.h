#pragma once

#include <stdint.h>

typedef struct EnvironmentApiContext {
    int argc;
    char **argv;
    void *platform_context;
} EnvironmentApiContext;

struct EnvironmentApi;

struct EnvironmentApi *environment_api_get_api(void);
int32_t environment_plugin_set_args(EnvironmentApiContext *context, int argc, char **argv, void *platform_context);