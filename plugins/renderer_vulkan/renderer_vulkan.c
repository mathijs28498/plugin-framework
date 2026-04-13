#include "renderer_vulkan.h"

#include <assert.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan, LOG_LEVEL_DEBUG);

#include "renderer_vulkan_register.h"
#include "renderer_vulkan_bootstrap.h"
#include "renderer_vulkan_utils.h"
#include "renderer_vulkan_start.h"

int32_t renderer_vulkan_cleanup(RendererContext *context)
{
    (void) renderer_vulkan_start_cleanup(context);
    TODO("Figure out what to do with the error code if something goes wrong");
    (void)renderer_vulkan_bootstrap_cleanup(context);

    return 0;
}
