#pragma once

#include <stdint.h>

struct EnvironmentContext;

int32_t environment_default_set_args(struct EnvironmentContext *context, int argc, char **argv, void *platform_context);
int32_t environment_default_get_platform_context(struct EnvironmentContext *context, void **platform_context);
int32_t environment_default_get_args(struct EnvironmentContext *context, int *argc, char ***argv);