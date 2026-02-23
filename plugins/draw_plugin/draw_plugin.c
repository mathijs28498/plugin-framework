#include "draw_plugin.h"

#include <stdint.h>
#include <assert.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(draw_plugin, LOG_LEVEL_DEBUG);

#include <plugin_manager_common.h>

#include "draw_plugin_register.h"

int32_t draw_plugin_present(DrawInterfaceContext *context)
{
    // return NOT_IMPLEMENTED(int32_t, context);
    (void) context;
    return 0;
}