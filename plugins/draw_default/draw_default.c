#include "draw_default.h"

#include <stdint.h>
#include <assert.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(draw_default, LOG_LEVEL_DEBUG);
#include <plugin_sdk/renderer/v1/renderer_interface.h>
#include <plugin_sdk/plugin_utils.h>

#include <plugin_sdk/plugin_utils.h>

#include "draw_default_register.h"

int32_t draw_default_start(DrawContext *context)
{
    assert(context != NULL);
    int32_t ret;

    RETURN_IF_ERROR(context->deps.logger, ret, renderer_start(context->deps.renderer),
                    "Error initializing renderer: %d", ret);

    return 0;
}

int32_t draw_default_present(DrawContext *context)
{
    assert(context != NULL);
    int32_t ret;

    RETURN_IF_ERROR(context->deps.logger, ret, renderer_render(context->deps.renderer),
                    "Error rendering renderer: %d", ret);
    return 0;
}

void draw_default_on_window_resize(DrawContext *context, uint32_t width, uint32_t height)
{
    assert(context != NULL);

    renderer_on_window_resize(context->deps.renderer, width, height);
}