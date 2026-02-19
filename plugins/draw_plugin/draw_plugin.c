#include "draw_plugin.h"

#include <stdint.h>

#include <logger_api.h>
LOGGER_API_REGISTER(draw_plugin, LOG_LEVEL_DEBUG);

#include "draw_plugin_register.h"

int32_t draw_plugin_present(DrawApiContext *context)
{
    LOG_INF(context->logger_api, "Drawing something!");
    return 0;
}