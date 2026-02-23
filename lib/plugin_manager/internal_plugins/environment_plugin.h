#pragma once

#include <stdint.h>

typedef struct EnvironmentInterfaceContext {
    int argc;
    char **argv;
    void *platform_context;
} EnvironmentInterfaceContext;

struct EnvironmentInterface;

struct EnvironmentInterface *environment_interface_get_interface(void);
int32_t environment_plugin_set_args(EnvironmentInterfaceContext *context, int argc, char **argv, void *platform_context);