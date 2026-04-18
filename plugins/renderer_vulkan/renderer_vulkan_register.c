#include "renderer_vulkan_register.h"

#include <plugin_sdk/renderer_interface.h>
#include <plugin_sdk/plugin_utils.h>

#include "renderer_vulkan.h"
#include "renderer_vulkan_start.h"
#include "renderer_vulkan_render.h"

static const RendererVtable plugin_vtable = {
    .start = renderer_vulkan_start,
    .render = renderer_vulkan_render,
    .on_window_resize = renderer_vulkan_on_window_resize,
};

int32_t plugin_init(RendererContext *context)
{
    (void)context;
    return 0;
}

int32_t plugin_shutdown(RendererContext *context)
{
    renderer_vulkan_cleanup(context);

    return 0;
}

#include "plugin_register.c.inc"