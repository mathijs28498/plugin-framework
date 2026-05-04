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

VkShaderStageFlags rv_shader_stage_to_vk_shader_stage(RendererShaderStage shader_stage)
{
    switch (shader_stage)
    {
    case RENDERER_SHADER_STAGE_VERTEX:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case RENDERER_SHADER_STAGE_FRAGMENT:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case RENDERER_SHADER_STAGE_COMPUTE:
        return VK_SHADER_STAGE_COMPUTE_BIT;
    default:
        return VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }
}

VkPipelineBindPoint rv_pipeline_type_to_vk_pipeline_bind_point(RendererPipelineType pipeline_type)
{
    switch (pipeline_type)
    {
    case RENDERER_PIPELINE_TYPE_GRAPHICS:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    case RENDERER_PIPELINE_TYPE_COMPUTE:
        return VK_PIPELINE_BIND_POINT_COMPUTE;
    default:
        return VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }
}

void renderer_vulkan_cmd_begin_render_pass(RendererContext *context, RendererCommandList *command_list)
{
    assert(context != NULL);
    assert(command_list != NULL);

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

    VkCommandBuffer cmd = command_list->command_buffer;

    vkCmdBeginRendering(cmd, &renderInfo);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void renderer_vulkan_cmd_end_render_pass(RendererContext *context, RendererCommandList *command_list)
{
    assert(context != NULL);
    assert(command_list != NULL);

    vkCmdEndRendering(command_list->command_buffer);
}

TODO("Create descriptor set handle")
void renderer_vulkan_cmd_bind_descriptor_sets(RendererContext *context, RendererCommandList *command_list, RendererPipelineType renderer_pipeline_type, RendererPipelineLayoutHandle pipeline_layout_handle)
{
    assert(context != NULL);
    assert(command_list != NULL);

    RendererVulkanHandle rv_pipeline_layout_handle = {.raw = pipeline_layout_handle};
    VkPipelineLayout pipeline_layout;
    RV_RES_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->pipeline_layouts, context->pipeline_layout_generations, rv_pipeline_layout_handle, pipeline_layout);

    vkCmdBindDescriptorSets(command_list->command_buffer, rv_pipeline_type_to_vk_pipeline_bind_point(renderer_pipeline_type), pipeline_layout, 0, 1, &context->draw_image_descriptor_set, 0, NULL);
}

TODO("Find a backend agnostic thing for this as push constants are vulkan, maybe a feature flag or something")
void renderer_vulkan_cmd_push_constants(RendererContext *context, RendererCommandList *command_list, RendererPipelineLayoutHandle pipeline_layout_handle, RendererShaderStage renderer_shader_stage, uint32_t offset, uint32_t push_constants_size, void *push_constants)
{
    assert(context != NULL);
    assert(command_list != NULL);

    RendererVulkanHandle rv_pipeline_layout_handle = {.raw = pipeline_layout_handle};
    VkPipelineLayout pipeline_layout;
    RV_RES_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->pipeline_layouts, context->pipeline_layout_generations, rv_pipeline_layout_handle, pipeline_layout);

    vkCmdPushConstants(command_list->command_buffer, pipeline_layout, rv_shader_stage_to_vk_shader_stage(renderer_shader_stage), offset, push_constants_size, push_constants);
}

void renderer_vulkan_cmd_dispatch(RendererContext *context, RendererCommandList *command_list, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)
{
    assert(context != NULL);
    assert(command_list != NULL);

    vkCmdDispatch(command_list->command_buffer, group_count_x, group_count_y, group_count_z);
}

void renderer_vulkan_cmd_bind_graphics_pipeline(RendererContext *context, RendererCommandList *command_list, RendererGraphicsPipelineHandle pipeline_handle)
{
    TODO("Actually bind the pipeline via the handle");
    assert(context != NULL);
    assert(command_list != NULL);
    RendererVulkanHandle renderer_pipeline_handle = {.raw = pipeline_handle};
    VkPipeline pipeline;
    RV_RES_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->pipelines, context->pipeline_generations, renderer_pipeline_handle, pipeline);
    vkCmdBindPipeline(command_list->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void renderer_vulkan_cmd_bind_compute_pipeline(RendererContext *context, RendererCommandList *command_list, RendererComputePipelineHandle pipeline_handle)
{
    assert(context != NULL);
    assert(command_list != NULL);
    RendererVulkanHandle renderer_pipeline_handle = {.raw = pipeline_handle};
    VkPipeline pipeline;
    RV_RES_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->pipelines, context->pipeline_generations, renderer_pipeline_handle, pipeline);
    vkCmdBindPipeline(command_list->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}

void renderer_vulkan_cmd_draw(RendererContext *context, RendererCommandList *command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
    assert(context != NULL);
    assert(command_list != NULL);
    vkCmdDraw(command_list->command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}