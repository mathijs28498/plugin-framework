#include "render_graph_default_register.h"

#include <plugin_sdk/render_graph/v1/render_graph_interface.h>
#include <plugin_sdk/plugin_utils.h>

#include "render_graph_default.h"


static const RenderGraphVtable plugin_vtable = {
    .register_pass = render_graph_default_register_pass,
    .render= render_graph_default_render,
};

#include "plugin_register.c.inc"