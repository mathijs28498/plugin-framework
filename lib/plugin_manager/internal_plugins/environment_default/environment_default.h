#pragma once

#include <stdint.h>

struct EnvironmentInterfaceContext;

int32_t environment_default_set_args(struct EnvironmentInterfaceContext *context, int argc, char **argv, void *platform_context);
int32_t environment_default_get_platform_context(struct EnvironmentInterfaceContext *context, void **platform_context);
int32_t environment_default_get_args(struct EnvironmentInterfaceContext *context, int *argc, char ***argv);