#include "renderer_vulkan_render.h"

#include <stdint.h>
#include <vulkan/vulkan.h>
#include <assert.h>
#include <math.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_render, LOG_LEVEL_DEBUG)

#include "renderer_vulkan_utils.h"
#include "renderer_vulkan_register.h"

#define SECOND_IN_NS 1000000000

VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspect_mask)
{
    return (VkImageSubresourceRange){
        .aspectMask = aspect_mask,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };
}

// ways to improve this efficiency: https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout)
{
    VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageMemoryBarrier2 image_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
        .oldLayout = current_layout,
        .newLayout = new_layout,

        .subresourceRange = image_subresource_range(aspect_mask),
        .image = image,
    };

    VkDependencyInfo dependency_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier,
    };

    vkCmdPipelineBarrier2(cmd, &dependency_info);
}

VkSemaphoreSubmitInfo create_semaphore_submit_info(VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore)
{
    return (VkSemaphoreSubmitInfo){
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = semaphore,
        .stageMask = stage_mask,
        .deviceIndex = 0,
        .value = 1,
    };
}

VkCommandBufferSubmitInfo create_command_buffer_submit_info(VkCommandBuffer cmd)
{
    return (VkCommandBufferSubmitInfo){
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
        .deviceMask = 0,
    };
}

VkSubmitInfo2 create_submit_info(VkCommandBufferSubmitInfo *cmd, VkSemaphoreSubmitInfo *signal_semaphore_info, VkSemaphoreSubmitInfo *wait_semaphore_info)
{
    return (VkSubmitInfo2){
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,

        .waitSemaphoreInfoCount = wait_semaphore_info == NULL ? 0 : 1,
        .pWaitSemaphoreInfos = wait_semaphore_info,

        .signalSemaphoreInfoCount = signal_semaphore_info == NULL ? 0 : 1,
        .pSignalSemaphoreInfos = signal_semaphore_info,

        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = cmd,
    };
}

int32_t renderer_vulkan_render(RendererContext *context)
{
    assert(context != NULL);

    VkResult result;
    RendererFrameData *frame = &context->frames[context->frame_number % ARRAY_SIZE(context->frames)];

    VK_RETURN_IF_ERROR(context->logger, result, vkWaitForFences(context->device, 1, &frame->render_fence, VK_TRUE, SECOND_IN_NS),
                       -1, "Failed to wait for render fence: %d", result);
    VK_RETURN_IF_ERROR(context->logger, result, vkResetFences(context->device, 1, &frame->render_fence),
                       -1, "Failed to reset render fence: %d", result);

    uint32_t swapchain_image_index;
    VK_RETURN_IF_ERROR(
        context->logger, result,
        vkAcquireNextImageKHR(context->device, context->swapchain, SECOND_IN_NS, frame->swapchain_semaphore, VK_NULL_HANDLE, &swapchain_image_index),
        -1, "Failed to acquire next image: %d", result);
    assert(swapchain_image_index < GET_ARRAY_LENGTH(context->swapchain_images));
    VkImage swapchain_image = context->swapchain_images[swapchain_image_index];

    VkCommandBuffer cmd = frame->main_command_buffer;

    VK_RETURN_IF_ERROR(context->logger, result, vkResetCommandBuffer(cmd, 0),
                       -1, "Failed to reset command buffer: %d", result);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VK_RETURN_IF_ERROR(context->logger, result, vkBeginCommandBuffer(cmd, &begin_info),
                       -1, "Failed to begin command buffer: %d", result);

    transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    TODO("Do something here")
    // float flash = sin((float)(context->frame_number) / 120.));
    float color_val = fabsf(sinf(context->frame_number / 120.f));
    VkClearColorValue clear_color = {{0., 0., color_val, 1.}};
    VkImageSubresourceRange clear_range = image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

    vkCmdClearColorImage(cmd, swapchain_image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &clear_range);

    transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK_RETURN_IF_ERROR(context->logger, result, vkEndCommandBuffer(cmd),
                       -1, "Failed to end buffer: %d", result);

    VkCommandBufferSubmitInfo cmd_info = create_command_buffer_submit_info(cmd);

    VkSemaphoreSubmitInfo wait_info = create_semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame->swapchain_semaphore);
    VkSemaphoreSubmitInfo signal_info = create_semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame->render_semaphore);

    VkSubmitInfo2 submit = create_submit_info(&cmd_info, &signal_info, &wait_info);

    VK_RETURN_IF_ERROR(context->logger, result, vkQueueSubmit2(context->graphics_queue, 1, &submit, frame->render_fence),
                       -1, "Failed to submit cmd to queue: %d", result);

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pSwapchains = &context->swapchain,
        .swapchainCount = 1,

        .pWaitSemaphores = &frame->render_semaphore,
        .waitSemaphoreCount = 1,

        .pImageIndices = &swapchain_image_index,
    };

    VK_RETURN_IF_ERROR(context->logger, result, vkQueuePresentKHR(context->present_queue, &present_info),
                       -1, "Failed to present queue: %d", result);

    context->frame_number++;

    return 0;
}