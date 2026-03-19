#include "environment_default.h"

#include <stdint.h>

#include "environment_default_register.h" 

int32_t environment_default_get_args(EnvironmentContext *context, int *argc, char ***argv)
{
    *argc = context->argc;
    *argv = context->argv;
    return 0;
}

int32_t environment_default_get_platform_context(EnvironmentContext *context, void **platform_context)
{
    *platform_context = context->platform_context;
    return 0;
}

int32_t environment_default_set_args(EnvironmentContext *context, int argc, char **argv, void *platform_context)
{
    context->argc = argc;
    context->argv = argv;
    context->platform_context = platform_context;
    return 0;
}