#include "renderer_vulkan.h"

#include <assert.h>
#include <vulkan/vulkan.h>

#include <plugin_sdk/logger_interface.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan, LOG_LEVEL_DEBUG);

#include "renderer_vulkan_register.h"
#include "renderer_vulkan_bootstrap.h"
#include "renderer_vulkan_utils.h"
#include "renderer_vulkan_start.h"

int32_t renderer_vulkan_cleanup(RendererContext *context)
{
    assert(context != NULL);

    if (context->device != VK_NULL_HANDLE)
    {
        VkResult result;
        VK_RETURN_IF_ERROR(context->logger, result, vkDeviceWaitIdle(context->device),
                           -1, "Failed to wait for device to idle: %d", result);
    }

    rv_call_queue_flush(&context->swapchain_destroy_queue);
    rv_call_queue_flush(&context->main_destroy_queue);

    return 0;
}

void renderer_vulkan_on_window_resize(RendererContext *context, uint32_t width, uint32_t height)
{
    assert(context != NULL);

    context->resize_requested = true;
    context->resize_extent.width = width;
    context->resize_extent.height = height;
}