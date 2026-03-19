#include "plugin_manager_default_register.h"

#include <plugin_manager_interface.h>
#include <plugin_manager_pm_interface.h>

#include "plugin_manager_default.h"

static const PluginManagerPMVtable plugin_vtable = {
    .public_vtable = {
        .get_singleton = plugin_manager_default_get_singleton,
    },
    .bootstrap = plugin_manager_default_bootstrap,
};

#include "plugin_register.c.inc"