#include "renderer_vulkan_cmd.h"

#include <stdint.h>
#include <vulkan/vulkan.h>
#include <assert.h>

#include "renderer_vulkan_register.h"
#include "renderer_vulkan_utils.h"

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

void renderer_vulkan_cmd_bind_pipeline(RendererContext *context, RendererCommandList *command_list, uint32_t pipeline_handle)
{
    TODO("Actually bind the pipeline via the handle");
    TODO("Create enum for pipeline type")
    assert(context != NULL);
    assert(command_list != NULL);
    (void) pipeline_handle;
    vkCmdBindPipeline(command_list->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->triangle_pipeline);
}
void renderer_vulkan_cmd_draw(RendererContext *context, RendererCommandList *command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
    assert(context != NULL);
    assert(command_list != NULL);
    vkCmdDraw(command_list->command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}