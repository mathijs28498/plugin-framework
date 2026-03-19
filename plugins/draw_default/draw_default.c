#include "draw_default.h"

#include <stdint.h>
#include <assert.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(draw_default, LOG_LEVEL_DEBUG);

#include <plugin_utils.h>

#include "draw_default_register.h"

int32_t draw_default_present(DrawContext *context)
{
    // return NOT_IMPLEMENTED(int32_t, context);
    (void) context;
    return 0;
}