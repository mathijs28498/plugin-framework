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

void renderer_vulkan_cmd_begin_render_pass(RendererContext *context, RendererCommandList *command_list)
{
    assert(context != NULL);
    assert(command_list != NULL);

    RV_AllocatedImage allocated_image = {0};
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->allocated_image_generations_a, context->allocated_images_a,
                                              context->draw_image_handle, allocated_image);

    VkRenderingAttachmentInfo color_attachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = allocated_image.image_view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };

    VkRenderingInfo rendering_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
            .extent = extent_2d(&context->draw_extent),
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
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

    VkCommandBuffer cmd = command_list->command_buffer;

    vkCmdBeginRendering(cmd, &rendering_info);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void renderer_vulkan_cmd_end_render_pass(RendererContext *context, RendererCommandList *command_list)
{
    assert(context != NULL);
    assert(command_list != NULL);

    vkCmdEndRendering(command_list->command_buffer);
}

void renderer_vulkan_cmd_bind_resource_sets(RendererContext *context, RendererCommandList *command_list, RendererPipelineType renderer_pipeline_type, RendererPipelineLayoutHandle pipeline_layout_handle, uint32_t first_set, uint32_t resource_set_len, const RendererResourceSetHandle *resource_set_handle, uint32_t dynamic_offset_len, const uint32_t *dynamic_offsets)
{
    assert(context != NULL);
    assert(command_list != NULL);

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->pipeline_layout_generations_a, context->pipeline_layouts_a, pipeline_layout_handle, pipeline_layout);

    TODO("Allow for multiple descriptor sets")
    VkDescriptorSet descriptor_set = context->active_frame_state.frame->transient_descriptor_sets[(size_t)resource_set_handle[0]];
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

void renderer_vulkan_cmd_draw(RendererContext *context, RendererCommandList *command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
    assert(context != NULL);
    assert(command_list != NULL);
    vkCmdDraw(command_list->command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

void renderer_vulkan_cmd_transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout)
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

        .subresourceRange = rv_image_subresource_range(aspect_mask),
        .image = image,
    };

    VkDependencyInfo dependency_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier,
    };

    vkCmdPipelineBarrier2(cmd, &dependency_info);
}