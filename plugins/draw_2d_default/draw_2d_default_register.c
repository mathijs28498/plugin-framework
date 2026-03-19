#include "draw_2d_default_register.h"

#include <draw_2d_interface.h>

#include "draw_2d_default.h"

static const Draw2dVtable plugin_vtable = {
    .test = draw_2d_test,
};

#include "plugin_register.c.inc"