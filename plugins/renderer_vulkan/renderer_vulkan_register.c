#include "renderer_vulkan_register.h"

#include <renderer_interface.h>

#include "renderer_vulkan.h"
#include "renderer_vulkan_start.h"
#include "renderer_vulkan_render.h"

static const RendererVtable plugin_vtable = {
    .start = renderer_vulkan_start,
    .render = renderer_vulkan_render,
}; 

int32_t plugin_init(RendererContext *context)
{
    context->frame_number = 0;

    return 0;
}

int32_t plugin_shutdown(RendererContext *context)
{
    renderer_vulkan_cleanup(context);

    return 0;
}

#include "plugin_register.c.inc"