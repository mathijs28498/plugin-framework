#include "draw_default.h"

#include <stdint.h>
#include <assert.h>

#include <cglm/types.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(draw_default, LOG_LEVEL_DEBUG);
#include <plugin_sdk/renderer/v1/renderer_interface.h>
#include <plugin_sdk/plugin_utils.h>

#include <plugin_sdk/plugin_utils.h>

#include "draw_default_register.h"
#include "draw_default_render_frame.h"



int32_t draw_default_present(DrawContext *context)
{
    assert(context != NULL);

    int32_t ret;
    LoggerInterface *logger = context->deps.logger;
    RendererInterface *renderer = context->deps.renderer;

    // Start the frame
    RendererCommandList *command_list;
    RETURN_IF_ERROR(logger, ret, renderer_begin_frame(renderer, &command_list),
                    "Failed to begin frame: %d", ret);

    if (ret == 1 || ret == 2)
    {
        return ret;
    }

    ret = draw_default_render_frame(context, command_list);
    if (ret < 0)
    {
        LOG_WRN_TRACE(logger, "Failed to render frame: %d", ret);
    }

    RETURN_IF_ERROR(logger, ret, renderer_end_frame(renderer),
                    "Failed to end frame: %d", ret);
    return 0;
}

void draw_default_on_window_resize(DrawContext *context, uint32_t width, uint32_t height)
{
    assert(context != NULL);

    renderer_on_window_resize(context->deps.renderer, width, height);
}
