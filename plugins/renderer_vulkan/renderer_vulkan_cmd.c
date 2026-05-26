#include "renderer_vulkan_cmd.h"

#include <stdint.h>
#include <vulkan/vulkan.h>
#include <assert.h>

#include <plugin_sdk/renderer/v1/renderer_interface.h>
#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_cmd, LOG_LEVEL_DEBUG)

#include "renderer_vulkan.h"
#include "renderer_vulkan_register.h"
#include "renderer_vulkan_utils.h"
#include "renderer_vulkan_conversion.h"

// void renderer_vulkan_cmd_begin_render_pass(RendererContext *context, RendererCommandList *command_list)
// {
//     assert(context != NULL);
//     assert(command_list != NULL);

//     RendererBeginRenderingInfo begin_rendering_info = {
//         .color_attachment_info = {
//             .load_op = RENDERER_ATTACHMENT_LOAD_OP_LOAD,
//             .store_op = RENDERER_ATTACHMENT_STORE_OP_STORE,
//             .image_handle = context->draw_image_handle,
//         }};

//     renderer_vulkan_cmd_begin_rendering(context, command_list, &begin_rendering_info);

//     RendererExtent2D renderer_draw_extent = {
//         .width = context->draw_extent.width,
//         .height = context->draw_extent.height};
//     renderer_vulkan_cmd_set_viewport(context, command_list, renderer_draw_extent);
//     renderer_vulkan_cmd_set_scissor(context, command_list, renderer_draw_extent);
// }

void renderer_vulkan_cmd_bind_resource_sets(RendererContext *context, RendererCommandList *command_list, RendererPipelineType renderer_pipeline_type, RendererPipelineLayoutHandle pipeline_layout_handle, uint32_t first_set, uint32_t resource_set_len, const RendererResourceSetHandle *resource_set_handle, uint32_t dynamic_offset_len, const uint32_t *dynamic_offsets)
{
    assert(context != NULL);
    assert(command_list != NULL);

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->pipeline_layout_generations_a, context->pipeline_layouts_a, pipeline_layout_handle, pipeline_layout);

    TODO("Allow for multiple descriptor sets")
    VkDescriptorSet descriptor_set = context->active_frame_state.frame->transient_descriptor_sets_a[(size_t)resource_set_handle[0]];
    vkCmdBindDescriptorSets(command_list->command_buffer, rv_pipeline_type_to_vk_pipeline_bind_point(renderer_pipeline_type), pipeline_layout, first_set, resource_set_len, &descriptor_set, dynamic_offset_len, dynamic_offsets);
}

TODO("Find a backend agnostic thing for this as push constants are vulkan, maybe a feature flag or something")
void renderer_vulkan_cmd_push_constants(RendererContext *context, RendererCommandList *command_list, RendererPipelineLayoutHandle pipeline_layout_handle, RendererShaderStageFlags renderer_shader_stage_flags, uint32_t offset, uint32_t push_constants_size, void *push_constants)
{
    assert(context != NULL);
    assert(command_list != NULL);

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->pipeline_layout_generations_a, context->pipeline_layouts_a, pipeline_layout_handle, pipeline_layout);

    vkCmdPushConstants(command_list->command_buffer, pipeline_layout, rv_shader_stage_to_vk_shader_stage(renderer_shader_stage_flags), offset, push_constants_size, push_constants);
}

void renderer_vulkan_cmd_dispatch(RendererContext *context, RendererCommandList *command_list, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)
{
    assert(context != NULL);
    assert(command_list != NULL);

    vkCmdDispatch(command_list->command_buffer, group_count_x, group_count_y, group_count_z);
}

void renderer_vulkan_cmd_bind_graphics_pipeline(RendererContext *context, RendererCommandList *command_list, RendererGraphicsPipelineHandle pipeline_handle)
{
    assert(context != NULL);
    assert(command_list != NULL);
    VkPipeline pipeline = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->pipeline_generations_a, context->pipelines_a, pipeline_handle, pipeline);
    vkCmdBindPipeline(command_list->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void renderer_vulkan_cmd_bind_compute_pipeline(RendererContext *context, RendererCommandList *command_list, RendererComputePipelineHandle pipeline_handle)
{
    assert(context != NULL);
    assert(command_list != NULL);
    VkPipeline pipeline = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->pipeline_generations_a, context->pipelines_a, pipeline_handle, pipeline);

    vkCmdBindPipeline(command_list->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}

void renderer_vulkan_cmd_bind_index_buffer(RendererContext *context, RendererCommandList *command_list, RendererBufferHandle buffer_handle)
{
    assert(context != NULL);
    assert(command_list != NULL);

    RV_AllocatedBuffer buffer = {0};
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->allocated_buffer_generations_a, context->allocated_buffers_a,
                                              buffer_handle, buffer);
    TODO("Figure out if other 2 should be arguments");
    vkCmdBindIndexBuffer(command_list->command_buffer, buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
}

void renderer_vulkan_cmd_draw_indexed(RendererContext *context, RendererCommandList *command_list, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance)
{
    assert(context != NULL);
    assert(command_list != NULL);

    vkCmdDrawIndexed(command_list->command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}

#include <cglm/cglm.h>
void renderer_vulkan_cmd_draw(RendererContext *context, RendererCommandList *command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
    assert(context != NULL);
    assert(command_list != NULL);
    vkCmdDraw(command_list->command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

// ways to improve this efficiency: https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
void renderer_vulkan_cmd_transition_image(RendererContext *context, RendererCommandList *command_list, RendererImageHandle image_handle, RendererImageLayout renderer_current_layout, RendererImageLayout renderer_new_layout)
{
    assert(context != NULL);

    RV_AllocatedImage allocated_image = {0};
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->allocated_image_generations_a, context->allocated_images_a, image_handle, allocated_image);

    VkImageLayout current_layout = rv_image_layout_to_vk_image_layout(renderer_current_layout);
    VkImageLayout new_layout = rv_image_layout_to_vk_image_layout(renderer_new_layout);

    TODO("Make this better, this is too fragile")
    VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageMemoryBarrier2 image_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
        .oldLayout = current_layout,
        .newLayout = new_layout,

        .subresourceRange = rv_image_subresource_range(aspect_mask),
        .image = allocated_image.image,
    };

    VkDependencyInfo dependency_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier,
    };

    vkCmdPipelineBarrier2(command_list->command_buffer, &dependency_info);
}

void renderer_vulkan_cmd_blit_image_to_image(RendererContext *context, RendererCommandList *command_list, RendererImageHandle image_handle_source, RendererImageHandle image_handle_destination, RendererExtent2D extent_source, RendererExtent2D extent_destination)
{
    assert(context != NULL);

    RV_AllocatedImage allocated_image_source = {0};
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->allocated_image_generations_a, context->allocated_images_a, image_handle_source, allocated_image_source);

    RV_AllocatedImage allocated_image_destination = {0};
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->allocated_image_generations_a, context->allocated_images_a, image_handle_destination, allocated_image_destination);

    VkOffset3D src_offset_max = {
        .x = extent_source.width,
        .y = extent_source.height,
        .z = 1,
    };

    VkOffset3D dst_offset_max = {
        .x = extent_destination.width,
        .y = extent_destination.height,
        .z = 1,
    };

    VkImageSubresourceLayers subresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseArrayLayer = 0,
        .layerCount = 1,
        .mipLevel = 0,
    };

    VkImageBlit2 blit_region = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
        .srcOffsets = {
            {.x = 0, .y = 0, .z = 0},
            src_offset_max,
        },
        .dstOffsets = {
            {.x = 0, .y = 0, .z = 0},
            dst_offset_max,
        },
        .srcSubresource = subresource,
        .dstSubresource = subresource,
    };

    VkBlitImageInfo2 blit_image_info = {
        .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
        .srcImage = allocated_image_source.image,
        .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .dstImage = allocated_image_destination.image,
        .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .filter = VK_FILTER_LINEAR,
        .regionCount = 1,
        .pRegions = &blit_region,
    };

    vkCmdBlitImage2(command_list->command_buffer, &blit_image_info);
}

void renderer_vulkan_cmd_begin_rendering(RendererContext *context, RendererCommandList *command_list, const RendererBeginRenderingInfo *renderer_begin_rendering_info)
{
    assert(context != NULL);
    assert(command_list != NULL);
    assert(renderer_begin_rendering_info != NULL);

    TODO("Allow for more options for attachments");

    RV_AllocatedImage color_attachment_image;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->allocated_image_generations_a, context->allocated_images_a,
                                              renderer_begin_rendering_info->color_attachment_info.image_handle, color_attachment_image);

    VkRenderingAttachmentInfo color_attachment_info = rv_attachment_info_to_vk_attachment_info(&renderer_begin_rendering_info->color_attachment_info, &color_attachment_image, RENDERER_ATTACHMENT_TYPE_COLOR);

    VkRenderingInfo rendering_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
            .extent = {
                .width = color_attachment_image.image_extent.width,
                .height = color_attachment_image.image_extent.height,
            }},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_info,
    };

    VkRenderingAttachmentInfo depth_attachment_info = {0};
    if (renderer_begin_rendering_info->depth_attachment_info != NULL)
    {
        RV_AllocatedImage depth_attachment_image;
        RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->allocated_image_generations_a, context->allocated_images_a,
                                                  renderer_begin_rendering_info->depth_attachment_info->image_handle, depth_attachment_image);

        depth_attachment_info = rv_attachment_info_to_vk_attachment_info(renderer_begin_rendering_info->depth_attachment_info, &depth_attachment_image, RENDERER_ATTACHMENT_TYPE_DEPTH);

        rendering_info.pDepthAttachment = &depth_attachment_info;
    }

    vkCmdBeginRendering(command_list->command_buffer, &rendering_info);
}

void renderer_vulkan_cmd_end_rendering(RendererContext *context, RendererCommandList *command_list)
{
    assert(context != NULL);
    assert(command_list != NULL);

    vkCmdEndRendering(command_list->command_buffer);
}

void renderer_vulkan_cmd_set_viewport(RendererContext *context, RendererCommandList *command_list, RendererExtent2D extent)
{
    assert(context != NULL);
    assert(command_list != NULL);

    VkViewport viewport = {
        .x = 0,
        .y = 0,
        .width = (float)extent.width,
        .height = (float)extent.height,
        .minDepth = 0,
        .maxDepth = 1,
    };

    vkCmdSetViewport(command_list->command_buffer, 0, 1, &viewport);
}

void renderer_vulkan_cmd_set_scissor(RendererContext *context, RendererCommandList *command_list, RendererExtent2D extent)
{
    assert(context != NULL);
    assert(command_list != NULL);

    VkRect2D scissor = {
        .offset = {
            .x = 0,
            .y = 0,
        },
        .extent = rv_renderer_extent_2d_to_vk_extent_2d(&extent),
    };

    vkCmdSetScissor(command_list->command_buffer, 0, 1, &scissor);
}