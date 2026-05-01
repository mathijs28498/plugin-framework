#include "renderer_vulkan_render.h"

#include <vk_mem_alloc.h>
#include <cglm/cglm.h>

#include <assert.h>
#include <stdint.h>
#include <vulkan/vulkan.h>
#include <assert.h>
#include <math.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
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
    assert(context != NULL);
    assert(cmd != VK_NULL_HANDLE);

    // float color_val = fabsf(sinf(context->frame_number / 120.f));
    // float color_val_1 = fabsf(cosf(context->frame_number / 120.f));
    // float color_val_2 = 1.f - fabsf(cosf(context->frame_number / 120.f));
    // VkClearColorValue clear_color = {{color_val_1, color_val_2, color_val, 1.}};
    // VkImageSubresourceRange clear_range = rv_image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

    // vkCmdClearColorImage(cmd, context->draw_image.image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &clear_range);

    ComputePushConstants push_constants = {
        .top_left = {1, 0, 0, 1},
        .top_right = {0, 0, 1, 1},
        .bottom_left = {0, 1, 1, 1},
        .bottom_right = {0, 1, 0, 1},
    };

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, context->gradient_pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, context->gradient_pipeline_layout, 0, 1, &context->draw_image_descriptor_set, 0, NULL);
    vkCmdPushConstants(cmd, context->gradient_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &push_constants);
    vkCmdDispatch(cmd, (int)ceil(context->draw_extent.width / 16.0), (int)ceil(context->draw_extent.height / 16.0), 1);
}

void draw_geometry(RendererContext *context, VkCommandBuffer cmd)
{
    assert(context != NULL);
    assert(cmd != VK_NULL_HANDLE);

    VkRenderingAttachmentInfo colorAttachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = context->draw_image.image_view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };

    VkRenderingInfo renderInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
            .extent = extent_2d(&context->draw_extent),
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment,
    };

    VkViewport viewport = {
        .x = 0,
        .y = 0,
        .width = (float)context->draw_extent.width,
        .height = (float)context->draw_extent.height,
    };
    VkRect2D scissor = {
        .offset.x = 0,
        .offset.y = 0,
        .extent = extent_2d(&context->draw_extent),
    };

    vkCmdBeginRendering(cmd, &renderInfo);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, context->triangle_pipeline);
    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, context->mesh_pipeline);
    GPUDrawPushConstants mesh_push_constants = {
        .world_matrix = GLM_MAT4_IDENTITY_INIT,
        .vertex_buffer_address = context->rectangle_mesh_buffers.vertex_buffer_address,
    };
    vkCmdPushConstants(cmd, context->mesh_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &mesh_push_constants);
    vkCmdBindIndexBuffer(cmd, context->rectangle_mesh_buffers.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);

    vkCmdEndRendering(cmd);
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

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkWaitForFences(context->device, 1, &frame->render_fence, VK_TRUE, SECOND_IN_NS),
                       -1, "Failed to wait for render fence: %d", result);

    rv_call_queue_flush(frame->destroy_queue);

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkResetFences(context->device, 1, &frame->render_fence),
                       -1, "Failed to reset render fence: %d", result);

    uint32_t swapchain_image_index;
    VK_RETURN_IF_ERROR_CONDITION(
        context->deps.logger, result, result < 0 && result != VK_ERROR_OUT_OF_DATE_KHR,
        vkAcquireNextImageKHR(context->device, context->swapchain, SECOND_IN_NS, frame->swapchain_semaphore, VK_NULL_HANDLE, &swapchain_image_index),
        -1, "Failed to acquire next image: %d", result);
    assert(swapchain_image_index < GET_ARRAY_LENGTH(context->swapchain_images));

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        renderer_vulkan_start_recreate_swapchain(context);
        TODO("Add proper enum values");
        LOG_WRN_TRACE(context->deps.logger, "Swapchain is out of date or suboptimal, aborting frame");
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

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkQueuePresentKHR(context->present_queue, &present_info),
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

    RETURN_IF_ERROR(context->deps.logger, ret, begin_frame(context, &frame, &swapchain_index),
                    "Failed to begin frame: %d", ret);

    if (ret == 1 || ret == 2)
    {
        return ret;
    }

    VkImage swapchain_image = context->swapchain_images[swapchain_index];

    VkCommandBuffer cmd = frame->main_command_buffer;

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkResetCommandPool(context->device, frame->command_pool, 0),
                       -1, "Failed to reset frame command pool: %d", result);

    VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    TODO("Check where this should be in the function");
    context->draw_extent.width = context->draw_image.image_extent.width;
    context->draw_extent.height = context->draw_image.image_extent.height;

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkBeginCommandBuffer(cmd, &cmd_begin_info),
                       -1, "Failed to begin command buffer: %d", result);

    rv_transition_image(cmd, context->draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    draw_background(context, cmd);

    rv_transition_image(cmd, context->draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    draw_geometry(context, cmd);

    rv_transition_image(cmd, context->draw_image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    rv_transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    rv_copy_image_to_image(cmd, context->draw_image.image, swapchain_image, extent_2d(&context->draw_extent), extent_2d(&context->swapchain_extent));

    rv_transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkEndCommandBuffer(cmd),
                       -1, "Failed to end buffer: %d", result);

    VkCommandBufferSubmitInfo cmd_info = rv_create_command_buffer_submit_info(cmd);

    VkSemaphoreSubmitInfo wait_info = rv_create_semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame->swapchain_semaphore);
    VkSemaphoreSubmitInfo signal_info = rv_create_semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame->render_semaphore);

    VkSubmitInfo2 submit = rv_create_submit_info(&cmd_info, &signal_info, &wait_info);

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkQueueSubmit2(context->graphics_queue, 1, &submit, frame->render_fence),
                       -1, "Failed to submit cmd to queue: %d", result);

    VK_RETURN_IF_ERROR(context->deps.logger, result, end_frame(context, frame, swapchain_index),
                       -1, "Failed to end frame: %d", result);

    return 0;
}