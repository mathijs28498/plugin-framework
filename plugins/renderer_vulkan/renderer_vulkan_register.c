#include "renderer_vulkan_register.h"

#include <renderer_interface.h>

#include "renderer_vulkan.h"

static const RendererVtable plugin_vtable = {
    .init = renderer_vulkan_init,
};

#include "plugin_register.c.inc"