#include "gui_application_default_register.h"

#include <gui_application_interface.h>

#include "gui_application_default.h"

static const GuiApplicationVtable plugin_vtable = {
    .setup = gui_application_default_setup,
    .run = gui_application_default_run,
};

#include "plugin_register.c.inc"