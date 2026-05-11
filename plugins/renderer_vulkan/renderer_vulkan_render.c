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
#include <plugin_sdk/renderer/v1/renderer_interface.h>

#include "renderer_vulkan_utils.h"
#include "renderer_vulkan_start.h"
#include "renderer_vulkan_register.h"
#include "renderer_vulkan_cmd.h"
#include "renderer_vulkan.h"

#define SECOND_IN_NS 1000000000

int32_t renderer_vulkan_render_begin_frame(RendererContext *context, RendererCommandList **out_command_list)
{
    assert(context != NULL);
    assert(out_command_list != NULL);
    assert(!context->active_frame_state.is_active);

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

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkWaitForFences(context->device, 1, &frame->render_fence, VK_TRUE, SECOND_IN_NS),
                       -1, "Failed to wait for render fence: %d", result);

    rv_call_queue_flush(frame->destroy_queue);

    uint32_t swapchain_image_index;
    RV_RETURN_IF_ERROR_CONDITION(
        context->deps.logger, result, result < 0 && result != VK_ERROR_OUT_OF_DATE_KHR,
        vkAcquireNextImageKHR(context->device, context->swapchain, SECOND_IN_NS, frame->swapchain_semaphore, VK_NULL_HANDLE, &swapchain_image_index),
        -1, "Failed to acquire next image: %d", result);
    assert(swapchain_image_index < GET_ARRAY_LENGTH(context->swapchain_images_a));

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        renderer_vulkan_start_recreate_swapchain(context);
        TODO("Add proper enum values");
        LOG_WRN_TRACE(context->deps.logger, "Swapchain is out of date or suboptimal, aborting frame");
        return 2;
    }

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkResetFences(context->device, 1, &frame->render_fence),
                       -1, "Failed to reset render fence: %d", result);

    VkCommandBuffer cmd = frame->command_list.command_buffer;

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkResetCommandPool(context->device, frame->command_pool, 0),
                       -1, "Failed to reset frame command pool: %d", result);

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkResetDescriptorPool(context->device, frame->transient_descriptor_pool, 0),
                       -1, "Failed to reset frame command pool: %d", result);
    GET_ARRAY_LENGTH(frame->transient_descriptor_sets) = 0;

    VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkBeginCommandBuffer(cmd, &cmd_begin_info),
                       -1, "Failed to begin command buffer: %d", result);

    context->active_frame_state.frame = frame;
    context->active_frame_state.swapchain_index = swapchain_image_index;
    context->active_frame_state.is_active = true;
    *out_command_list = &frame->command_list;

    TODO("Also abstract this away somehow, possibly with different passes");

    RV_AllocatedImage allocated_image = {0};
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->allocated_image_generations_a, context->allocated_images_a,
                                         context->draw_image_handle, allocated_image);
    rv_transition_image(cmd, allocated_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    return 0;
}

int32_t renderer_vulkan_render_end_frame(RendererContext *context)
{
    assert(context != NULL);
    assert(context->active_frame_state.frame != NULL);

    VkResult result;

    RendererFrameData *frame = context->active_frame_state.frame;
    VkCommandBuffer cmd = frame->command_list.command_buffer;
    VkImage swapchain_image = context->swapchain_images_a[context->active_frame_state.swapchain_index];

    TODO("Remove this and add render passes");
    // rv_transition_image(cmd, context->draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    RV_AllocatedImage allocated_image = {0};
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->allocated_image_generations_a, context->allocated_images_a,
                                              context->draw_image_handle, allocated_image);

    rv_transition_image(cmd, allocated_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    rv_transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    rv_copy_image_to_image(cmd, allocated_image.image, swapchain_image, extent_2d(&context->draw_extent), extent_2d(&context->swapchain_extent));

    rv_transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkEndCommandBuffer(cmd),
                       -1, "Failed to end buffer: %d", result);

    VkCommandBufferSubmitInfo cmd_info = rv_create_command_buffer_submit_info(cmd);

    VkSemaphoreSubmitInfo wait_info = rv_create_semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame->swapchain_semaphore);
    VkSemaphoreSubmitInfo signal_info = rv_create_semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame->render_semaphore);

    VkSubmitInfo2 submit = rv_create_submit_info(&cmd_info, &signal_info, &wait_info);

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkQueueSubmit2(context->graphics_queue, 1, &submit, frame->render_fence),
                       -1, "Failed to submit cmd to queue: %d", result);

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pSwapchains = &context->swapchain,
        .swapchainCount = 1,

        .pWaitSemaphores = &context->active_frame_state.frame->render_semaphore,
        .waitSemaphoreCount = 1,

        .pImageIndices = &context->active_frame_state.swapchain_index,
    };
    context->active_frame_state.is_active = false;

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkQueuePresentKHR(context->present_queue, &present_info),
                       -1, "Failed to present queue: %d", result);

    context->frame_number++;

    return 0;
}

void draw_geometry(RendererContext *context, VkCommandBuffer cmd)
{
    assert(context != NULL);
    assert(cmd != VK_NULL_HANDLE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, context->triangle_pipeline);
    vkCmdDraw(cmd, 3, 1, 0, 0);

    // vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, context->mesh_pipeline);
    // GPUDrawPushConstants mesh_push_constants = {
    //     .world_matrix = GLM_MAT4_IDENTITY_INIT,
    //     .vertex_buffer_address = context->rectangle_mesh_buffers.vertex_buffer_address,
    // };
    // vkCmdPushConstants(cmd, context->mesh_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &mesh_push_constants);
    // vkCmdBindIndexBuffer(cmd, context->rectangle_mesh_buffers.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    // vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);
}
