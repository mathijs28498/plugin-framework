#include "renderer_vulkan_render.h"

#include <vk_mem_alloc.h>

#include <stdint.h>
#include <vulkan/vulkan.h>
#include <assert.h>
#include <math.h>

#include <plugin_sdk/logger_interface.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_render, LOG_LEVEL_DEBUG)

#include "renderer_vulkan_utils.h"
#include "renderer_vulkan_start.h"
#include "renderer_vulkan_register.h"

#define SECOND_IN_NS 1000000000

VkExtent2D extent_2d(RV_VkExtent2D *rv_extent)
{
    return (VkExtent2D){
        .width = rv_extent->width,
        .height = rv_extent->height,
    };
}

void draw_background(RendererContext *context, VkCommandBuffer cmd)
{
    float color_val = fabsf(sinf(context->frame_number / 120.f));
    float color_val_1 = fabsf(cosf(context->frame_number / 120.f));
    float color_val_2 = 1.f - fabsf(cosf(context->frame_number / 120.f));
    VkClearColorValue clear_color = {{color_val_1, color_val_2, color_val, 1.}};
    VkImageSubresourceRange clear_range = rv_image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

    vkCmdClearColorImage(cmd, context->draw_image.image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &clear_range);
}

TODO("Stop renderering if resize extent becomes 0, 0")
int32_t begin_frame(RendererContext *context, RendererFrameData **out_frame, uint32_t *out_swapchain_index)
{
    assert(context != NULL);
    assert(out_frame != NULL);
    assert(out_swapchain_index != NULL);

    VkResult result;

    if (context->resize_requested)
    {
        renderer_vulkan_start_recreate_swapchain(context);
    }

    if (context->halt_render)
    {
        TODO("Add proper enum values");
        return 1;
    }

    RendererFrameData *frame = &context->frames[context->frame_number % ARRAY_SIZE(context->frames)];

    VK_RETURN_IF_ERROR(context->logger, result, vkWaitForFences(context->device, 1, &frame->render_fence, VK_TRUE, SECOND_IN_NS),
                       -1, "Failed to wait for render fence: %d", result);

    rv_call_queue_flush(&frame->destroy_queue);

    VK_RETURN_IF_ERROR(context->logger, result, vkResetFences(context->device, 1, &frame->render_fence),
                       -1, "Failed to reset render fence: %d", result);

    uint32_t swapchain_image_index;
    VK_RETURN_IF_ERROR_CONDITION(
        context->logger, result, result < 0 && result != VK_ERROR_OUT_OF_DATE_KHR,
        vkAcquireNextImageKHR(context->device, context->swapchain, SECOND_IN_NS, frame->swapchain_semaphore, VK_NULL_HANDLE, &swapchain_image_index),
        -1, "Failed to acquire next image: %d", result);
    assert(swapchain_image_index < GET_ARRAY_LENGTH(context->swapchain_images));

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        renderer_vulkan_start_recreate_swapchain(context);
        TODO("Add proper enum values");
        LOG_WRN(context->logger, "Swapchain is out of date or suboptimal, aborting frame");
        return 2;
    }

    *out_frame = &context->frames[context->frame_number % ARRAY_SIZE(context->frames)];
    *out_swapchain_index = swapchain_image_index;

    return 0;
}

int32_t end_frame(RendererContext *context, RendererFrameData *frame, uint32_t swapchain_index)
{
    assert(context != NULL);
    assert(frame != NULL);

    VkResult result;

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pSwapchains = &context->swapchain,
        .swapchainCount = 1,

        .pWaitSemaphores = &frame->render_semaphore,
        .waitSemaphoreCount = 1,

        .pImageIndices = &swapchain_index,
    };

    VK_RETURN_IF_ERROR(context->logger, result, vkQueuePresentKHR(context->present_queue, &present_info),
                       -1, "Failed to present queue: %d", result);

    context->frame_number++;

    return 0;
}

int32_t renderer_vulkan_render(RendererContext *context)
{
    assert(context != NULL);
    VkResult result;
    uint32_t ret;

    RendererFrameData *frame;
    uint32_t swapchain_index;

    RETURN_IF_ERROR(context->logger, ret, begin_frame(context, &frame, &swapchain_index),
                    "Failed to begin frame: %d", ret);

    if (ret == 1 || ret == 2)
    {
        return ret;
    }

    VkImage swapchain_image = context->swapchain_images[swapchain_index];

    VkCommandBuffer cmd = frame->main_command_buffer;

    VK_RETURN_IF_ERROR(context->logger, result, vkResetCommandBuffer(cmd, 0),
                       -1, "Failed to reset command buffer: %d", result);

    VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    context->draw_extent.width = context->draw_image.image_extent.width;
    context->draw_extent.height = context->draw_image.image_extent.height;

    VK_RETURN_IF_ERROR(context->logger, result, vkBeginCommandBuffer(cmd, &cmd_begin_info),
                       -1, "Failed to begin command buffer: %d", result);

    rv_transition_image(cmd, context->draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    draw_background(context, cmd);

    rv_transition_image(cmd, context->draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    rv_transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    rv_copy_image_to_image(cmd, context->draw_image.image, swapchain_image, extent_2d(&context->draw_extent), extent_2d(&context->swapchain_extent));

    rv_transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK_RETURN_IF_ERROR(context->logger, result, vkEndCommandBuffer(cmd),
                       -1, "Failed to end buffer: %d", result);

    VkCommandBufferSubmitInfo cmd_info = rv_create_command_buffer_submit_info(cmd);

    VkSemaphoreSubmitInfo wait_info = rv_create_semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame->swapchain_semaphore);
    VkSemaphoreSubmitInfo signal_info = rv_create_semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame->render_semaphore);

    VkSubmitInfo2 submit = rv_create_submit_info(&cmd_info, &signal_info, &wait_info);

    VK_RETURN_IF_ERROR(context->logger, result, vkQueueSubmit2(context->graphics_queue, 1, &submit, frame->render_fence),
                       -1, "Failed to submit cmd to queue: %d", result);

    VK_RETURN_IF_ERROR(context->logger, result, end_frame(context, frame, swapchain_index),
                       -1, "Failed to end frame: %d", result);

    return 0;
}