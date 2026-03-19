#pragma once

#include <stdint.h>

#include "environment_interface.h"

#pragma pack(push, 8)

struct EnvironmentContext;

typedef struct EnvironmentPMVtable
{
    EnvironmentVtable public_vtable;

    int32_t (*set_args)(struct EnvironmentContext *context, int argc, char **argv, void *platform_context);
} EnvironmentPMVtable;

#pragma pack(pop)

static inline int32_t environment_pm_set_args(EnvironmentInterface *iface, int argc, char **argv, void *platform_context)
{
    return ((EnvironmentPMVtable *)iface->vtable)->set_args(iface->context, argc, argv, platform_context);
}