#include "draw_default_register.h"

#include <draw_interface.h>

#include "draw_default.h"

static const DrawVtable plugin_vtable = {
    .present = draw_default_present,
};

#include "plugin_register.c.inc"