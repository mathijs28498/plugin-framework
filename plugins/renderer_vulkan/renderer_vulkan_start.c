#include "renderer_vulkan_start.h"

#include <stdint.h>
#include <vulkan/vulkan.h>
#include <assert.h>

#include <plugin_utils.h>
#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_start, LOG_LEVEL_WARNING)

#include "renderer_vulkan_bootstrap.h"
#include "renderer_vulkan_utils.h"
#include "renderer_vulkan_register.h"

int32_t init_commands(RendererContext *context)
{
    assert(context != NULL);
    VkResult result;

    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = context->graphics_queue_family,
    };

    VkCommandBufferAllocateInfo command_buffer_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandBufferCount = 1,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };

    for (size_t i = 0; i < ARRAY_SIZE(context->frames); i++)
    {
        RendererFrameData *frame = &context->frames[i];
        VK_RETURN_IF_ERROR(context->logger, result,
                           vkCreateCommandPool(context->device, &command_pool_create_info, NULL, &frame->command_pool),
                           -1, "Failed to create command pool for frame %d: %d", i, result);

        command_buffer_alloc_info.commandPool = frame->command_pool;

        VK_RETURN_IF_ERROR(context->logger, result,
                           vkAllocateCommandBuffers(context->device, &command_buffer_alloc_info, &frame->main_command_buffer),
                           -1, "Failed to allocate command buffer for frame %d: %d", i, result);
    }

    return 0;
}

int32_t create_sync_structures(RendererContext *context)
{
    assert(context != NULL);

    VkResult result;

    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    for (size_t i = 0; i < ARRAY_SIZE(context->frames); i++)
    {
        RendererFrameData *frame = &context->frames[i];
        VK_RETURN_IF_ERROR(context->logger, result, vkCreateFence(context->device, &fence_create_info, NULL, &frame->render_fence),
                           -1, "Failed to create fence for frame %d: %d", i, result);

        VK_RETURN_IF_ERROR(context->logger, result, vkCreateSemaphore(context->device, &semaphore_create_info, NULL, &frame->render_semaphore),
                           -1, "Failed to create render semaphore for frame %d: %d", i, result);

        VK_RETURN_IF_ERROR(context->logger, result, vkCreateSemaphore(context->device, &semaphore_create_info, NULL, &frame->swapchain_semaphore),
                           -1, "Failed to create swacphain semaphore for frame %d: %d", i, result);
    }

    return 0;
}

int32_t renderer_vulkan_start(RendererContext *context)
{
    assert(context != NULL);

    int32_t ret;
    VK_TRY_INIT(context->logger, ret, renderer_vulkan_bootstrap(context), (void)renderer_vulkan_start_cleanup(context),
                "Failed to bootstrap vulkan: %d", ret);

    VK_TRY_INIT(context->logger, ret, init_commands(context), (void)renderer_vulkan_start_cleanup(context),
                "Failed to init commands: %d", ret);

    VK_TRY_INIT(context->logger, ret, create_sync_structures(context), (void)renderer_vulkan_start_cleanup(context),
                "Failed to create sync structures: %d", ret);

    return 0;
}

int32_t renderer_vulkan_start_cleanup(RendererContext *context)
{
    assert(context != NULL);
    assert(context->device != VK_NULL_HANDLE);

    RETURN_IF_FALSE(context->logger, context->device != VK_NULL_HANDLE, -1,
                    "Device is uninitialized when trying to cleanup renderer vulkan start");

    VkResult result;

    VK_RETURN_IF_ERROR(context->logger, result, vkDeviceWaitIdle(context->device),
                       -1, "Failed to wait for device to idle: %d", result);

    for (size_t i = 0; i < ARRAY_SIZE(context->frames); i++)
    {
        RendererFrameData *frame = &context->frames[i];
        VK_DESTROY_CHECKED(frame->command_pool, vkDestroyCommandPool(context->device, frame->command_pool, NULL));

        VK_DESTROY_CHECKED(frame->render_fence, vkDestroyFence(context->device, frame->render_fence, NULL));
        VK_DESTROY_CHECKED(frame->render_semaphore, vkDestroySemaphore(context->device, frame->render_semaphore, NULL));
        VK_DESTROY_CHECKED(frame->swapchain_semaphore, vkDestroySemaphore(context->device, frame->swapchain_semaphore, NULL));
    }
    return 0;
}