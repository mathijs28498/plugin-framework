#include "draw_default.h"

#include <stdint.h>
#include <assert.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(draw_default, LOG_LEVEL_DEBUG);
#include <renderer_interface.h>
#include <plugin_utils.h>

#include <plugin_utils.h>

#include "draw_default_register.h"

int32_t draw_default_start(DrawContext *context)
{
    int32_t ret;

    RETURN_IF_ERROR(context->logger, ret, renderer_start(context->renderer),
                    "Error initializing renderer: %d", ret);

    return 0;
}

int32_t draw_default_present(DrawContext *context)
{
    int32_t ret;

    RETURN_IF_ERROR(context->logger, ret, renderer_render(context->renderer),
                    "Error rendering renderer: %d", ret);
    return 0;
}