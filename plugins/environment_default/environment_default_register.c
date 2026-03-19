#include "environment_default_register.h"

#include <environment_interface.h>
#include <environment_pm_interface.h>

#include "environment_default.h"

static const EnvironmentPMVtable plugin_vtable = {

    .public_vtable = {
        .get_args = environment_default_get_args,
        .get_platform_context = environment_default_get_platform_context,
    },
    .set_args = environment_default_set_args,
};

#include "plugin_register.c.inc"