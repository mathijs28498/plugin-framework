#include "render_graph_default.h"

#include <stdint.h>
#include <assert.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(render_graph_default, LOG_LEVEL_DEBUG)
#include <plugin_sdk/render_graph/v1/render_graph_interface.h>

#include "render_graph_default_register.h"

int32_t render_graph_default_register_pass(RenderGraphContext *context, RG_Pass *pass)
{
    assert(context != NULL);
    assert(pass != NULL);

    return 0;
}

int32_t render_graph_default_render(RenderGraphContext *context)
{
    assert(context != NULL);

    return 0;
}