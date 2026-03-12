#include "draw_2d_default_register.h"

#include <draw_2d_interface.h>

#include "draw_2d_default.h"

static Draw2dInterface *get_interface(void)
{
    static Draw2dInterfaceContext context = {0};

    static Draw2dInterface iface = {
        .context = &context,

        .test = draw_2d_test,
    };

    return &iface;
}

#include "plugin_register.c.inc"