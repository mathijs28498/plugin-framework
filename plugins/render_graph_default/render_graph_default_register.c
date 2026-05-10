#include "render_graph_default_register.h"

#include <plugin_sdk/render_graph/v1/render_graph_interface.h>
#include <plugin_sdk/plugin_utils.h>

#include "render_graph_default.h"

void dummy(void)
{
}

static const RenderGraphVtable plugin_vtable = {
    .dummy = dummy,
};

#include "plugin_register.c.inc"