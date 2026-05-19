#include "draw_default_register.h"

#include <plugin_sdk/draw/v1/draw_interface.h>

#include "draw_default.h"
#include "draw_default_start.h"

static const DrawVtable plugin_vtable = {
    .start = draw_default_start,
    .present = draw_default_present,
    .on_window_resize = draw_default_on_window_resize,
};

static int32_t plugin_shutdown(DrawContext *context)
{
    draw_default_cleanup(context);

    return 0;
}

#include "plugin_register.c.inc"