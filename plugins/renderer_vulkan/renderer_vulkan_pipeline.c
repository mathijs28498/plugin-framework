#include "renderer_vulkan_pipeline.h"

#include <plugin_sdk/plugin_utils.h>

#include <assert.h>
#include <stdint.h>
#include <vulkan/vulkan.h>
TODO("Remove this dependency")
#include <stdlib.h>
#include <stdbool.h>

#include <bump_arena.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_pipeline, LOG_LEVEL_DEBUG)
#include <plugin_sdk/renderer/v1/renderer_interface.h>

#include "renderer_vulkan_register.h"
#include "renderer_vulkan_utils.h"
#include "renderer_vulkan.h"
#include "renderer_vulkan_conversion.h"

int32_t renderer_vulkan_create_shader(RendererContext *context, const uint32_t *shader_code_u32, size_t shader_code_bytes_len, RendererShaderHandle *out_shader_handle)
{
    assert(context != NULL);
    assert(out_shader_handle != NULL);

    VkResult result;

    VkShaderModuleCreateInfo shader_module_create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pCode = shader_code_u32,
        .codeSize = shader_code_bytes_len,
    };

    VkShaderModule shader_module;
    RV_RETURN_IF_ERROR(
        context->deps.logger, result, vkCreateShaderModule(context->device, &shader_module_create_info, NULL, &shader_module),
        -1, "Failed to create shader module: %d", result);

    RendererVulkanHandle shader_handle = {0};
    RV_RES_RV_HANDLE_ALLOC_OR_RETURN(context->deps.logger, context->shader_module_occupied_a, context->shader_module_generations_a, context->shader_modules_a, shader_module, shader_handle,
                                     vkDestroyShaderModule(context->device, shader_module, NULL));
    *out_shader_handle = shader_handle.raw;
    return 0;
}

static inline int32_t inl_renderer_vulkan_create_pipeline_layout(RendererContext *context, const RendererPipelineLayoutCreateInfo *renderer_pipeline_layout_create_info, RendererPipelineLayoutHandle *out_pipeline_layout_handle)
{
    VkResult result;
    int32_t ret;

    uint32_t push_constant_ranges_len = renderer_pipeline_layout_create_info->push_constants_len;
    VkPushConstantRange *push_constant_ranges;
    RETURN_IF_ERROR(context->deps.logger, ret, BUMP_ARENA_ALLOC_TYPED(context->bump_arena_a, VkPushConstantRange, push_constant_ranges_len, &push_constant_ranges),
                    "Failed to allocate from bump arena: %d", ret);

    for (size_t i = 0; i < push_constant_ranges_len; i++)
    {
        RendererPushConstantsInfo *push_constants_info = &renderer_pipeline_layout_create_info->push_constants[i];
        push_constant_ranges[i].stageFlags = rv_shader_stage_to_vk_shader_stage(push_constants_info->render_stage_flags);
        push_constant_ranges[i].offset = push_constants_info->offset;
        push_constant_ranges[i].size = push_constants_info->size;
    }

    uint32_t descriptor_set_layouts_len = renderer_pipeline_layout_create_info->resource_set_layout_handles_len;
    VkDescriptorSetLayout *descriptor_set_layouts;
    RETURN_IF_ERROR(context->deps.logger, ret, BUMP_ARENA_ALLOC_TYPED(context->bump_arena_a, VkDescriptorSetLayout, descriptor_set_layouts_len, &descriptor_set_layouts),
                    "Failed to allocate from bump arena: %d", ret);
    for (size_t i = 0; i < descriptor_set_layouts_len; i++)
    {
        TODO("Make this behave properly");
        RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->descriptor_set_layout_generations_a, context->descriptor_set_layouts_a,
                                             renderer_pipeline_layout_create_info->resource_set_layout_handles[i], descriptor_set_layouts[i]);
    }

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = descriptor_set_layouts_len,
        .pSetLayouts = descriptor_set_layouts,
        .pushConstantRangeCount = push_constant_ranges_len,
        .pPushConstantRanges = push_constant_ranges,
    };

    VkPipelineLayout pipeline_layout;
    RV_RETURN_IF_ERROR(context->deps.logger, result, vkCreatePipelineLayout(context->device, &pipeline_layout_create_info, NULL, &pipeline_layout),
                       -1, "Failed to create pipeline layout: %d", result);

    RendererVulkanHandle rv_pipeline_layout_handle = {0};
    RV_RES_RV_HANDLE_ALLOC_OR_RETURN(context->deps.logger, context->pipeline_layout_occupied_a, context->pipeline_layout_generations_a, context->pipeline_layouts_a, pipeline_layout, rv_pipeline_layout_handle,
                                     vkDestroyPipelineLayout(context->device, pipeline_layout, NULL));

    *out_pipeline_layout_handle = rv_pipeline_layout_handle.raw;
    return 0;
}

int32_t renderer_vulkan_create_pipeline_layout(RendererContext *context, const RendererPipelineLayoutCreateInfo *renderer_pipeline_layout_create_info, RendererPipelineLayoutHandle *out_pipeline_layout_handle)
{
    assert(context != NULL);
    assert(renderer_pipeline_layout_create_info != NULL);
    assert(out_pipeline_layout_handle != NULL);

    BumpArenaCheckpoint bump_arena_checkpoint = bump_arena_create_checkpoint(context->bump_arena_a);
    int32_t ret = inl_renderer_vulkan_create_pipeline_layout(context, renderer_pipeline_layout_create_info, out_pipeline_layout_handle);
    bump_arena_restore_checkpoint(context->bump_arena_a, bump_arena_checkpoint, true);

    return ret;
}

int32_t rv_create_pipeline_shader_stage_create_info(RendererContext *context, const RendererShaderCreateInfo *shader_create_info, VkShaderStageFlagBits shader_stage, VkPipelineShaderStageCreateInfo *out_pipeline_shader_stage_create_info)
{
    assert(context != NULL);
    assert(shader_create_info != NULL);
    assert(out_pipeline_shader_stage_create_info != NULL);

    VkShaderModule shader_module = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->shader_module_generations_a, context->shader_modules_a,
                                         shader_create_info->shader_handle, shader_module);

    *out_pipeline_shader_stage_create_info = (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = shader_stage,
        .module = shader_module,
        .pName = shader_create_info->entry_point,
    };

    return 0;
}

VkPipelineColorBlendAttachmentState create_color_blend_attachment_state(RendererBlendMode blend_mode)
{
    switch (blend_mode)
    {
    case RENDERER_BLEND_MODE_NONE:
        return (VkPipelineColorBlendAttachmentState){
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            .blendEnable = VK_FALSE,
        };
    case RENDERER_BLEND_MODE_ADDITIVE:
        return (VkPipelineColorBlendAttachmentState){
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
        };
    case RENDERER_BLEND_MODE_ALPHA_BLEND:
        return (VkPipelineColorBlendAttachmentState){
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
        };
    default:
        UNREACHABLE();
        return (VkPipelineColorBlendAttachmentState){0};
    }
}

int32_t renderer_vulkan_create_graphics_pipeline(RendererContext *context, const RendererGraphicsPipelineCreateInfo *renderer_pipeline_create_info, RendererGraphicsPipelineHandle *out_pipeline_handle)
{
    assert(context != NULL);
    assert(renderer_pipeline_create_info != NULL);
    assert(out_pipeline_handle != NULL);

    int32_t ret;
    VkResult result;

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->pipeline_layout_generations_a, context->pipeline_layouts_a,
                                         renderer_pipeline_create_info->layout_handle, pipeline_layout);

    VkFormat vk_color_attachment_format = rv_image_format_to_vk_format(renderer_pipeline_create_info->color_attachment_format);
    VkFormat vk_depth_attachment_format = rv_image_format_to_vk_format(renderer_pipeline_create_info->depth_attachment_format);
    VkPipelineRenderingCreateInfo rendering_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &vk_color_attachment_format,
        .depthAttachmentFormat = vk_depth_attachment_format,
    };

    VkPipelineShaderStageCreateInfo vertex_stage_create_info;
    VkPipelineShaderStageCreateInfo fragment_stage_create_info;

    RETURN_IF_ERROR(context->deps.logger, ret, rv_create_pipeline_shader_stage_create_info(context, &renderer_pipeline_create_info->vertex_shader, VK_SHADER_STAGE_VERTEX_BIT, &vertex_stage_create_info),
                    "Failed to create shader vertex stage create info: %d", ret);
    RETURN_IF_ERROR(context->deps.logger, ret, rv_create_pipeline_shader_stage_create_info(context, &renderer_pipeline_create_info->fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT, &fragment_stage_create_info),
                    "Failed to create shader fragment stage create info: %d", ret);

    VkPipelineShaderStageCreateInfo shader_stage_create_infos[] = {
        vertex_stage_create_info,
        fragment_stage_create_info,
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = rv_topology_to_vk_topology(renderer_pipeline_create_info->topology),
        .primitiveRestartEnable = VK_FALSE,
    };

    VkPipelineViewportStateCreateInfo viewport_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterization_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = rv_fill_mode_to_vk_polygon_mode(renderer_pipeline_create_info->fill_mode),
        .lineWidth = 1,
        .cullMode = rv_cull_mode_to_vk_cull_mode(renderer_pipeline_create_info->cull_mode),
        .frontFace = rv_front_face_to_vk_front_face(renderer_pipeline_create_info->front_face),
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = create_color_blend_attachment_state(renderer_pipeline_create_info->blend_mode);

    VkPipelineColorBlendStateCreateInfo color_blend_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment_state,
    };

    TODO("Add support for different types of multisampling, this is no sampling")
    VkPipelineMultisampleStateCreateInfo multisample_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,

        // multisampling defaulted to no multisampling (1 sample per pixel)
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,

        // no alpha to coverage either
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    TODO("Add support for different depth tests, this is disabled depth test")
    VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_NEVER,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .minDepthBounds = 0,
        .maxDepthBounds = 1,
    };

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = ARRAY_SIZE(dynamic_states),
        .pDynamicStates = dynamic_states,
    };

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &rendering_create_info,

        .stageCount = ARRAY_SIZE(shader_stage_create_infos),
        .pStages = shader_stage_create_infos,

        .pVertexInputState = &vertex_input_create_info,
        .pInputAssemblyState = &input_assembly_create_info,
        .pTessellationState = NULL,
        .pViewportState = &viewport_create_info,
        .pRasterizationState = &rasterization_create_info,
        .pMultisampleState = &multisample_create_info,
        .pDepthStencilState = &depth_stencil_create_info,
        .pColorBlendState = &color_blend_create_info,
        .pDynamicState = &dynamic_state_create_info,

        .layout = pipeline_layout,
    };

    VkPipeline pipeline;
    RV_RETURN_IF_ERROR(context->deps.logger, result,
                       vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, NULL, &pipeline),
                       -1, "Failed to create graphics pipeline: %d", result);

    RendererVulkanHandle pipeline_handle = {0};
    RV_RES_RV_HANDLE_ALLOC_OR_RETURN(context->deps.logger, context->pipeline_occupied_a, context->pipeline_generations_a, context->pipelines_a, pipeline, pipeline_handle,
                                     vkDestroyPipeline(context->device, pipeline, NULL));

    *out_pipeline_handle = pipeline_handle.raw;
    return 0;
}

int32_t renderer_vulkan_create_compute_pipeline(RendererContext *context, const RendererComputePipelineCreateInfo *renderer_pipeline_create_info, RendererComputePipelineHandle *out_pipeline_handle)
{
    assert(context != NULL);
    assert(renderer_pipeline_create_info != NULL);
    assert(out_pipeline_handle != NULL);

    VkResult result;
    int32_t ret;

    VkPipelineShaderStageCreateInfo stage_create_info;
    RETURN_IF_ERROR(context->deps.logger, ret, rv_create_pipeline_shader_stage_create_info(context, &renderer_pipeline_create_info->compute_shader, VK_SHADER_STAGE_COMPUTE_BIT, &stage_create_info),
                    "Failed to create compute shader stage create info: %d", ret);

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->pipeline_layout_generations_a, context->pipeline_layouts_a,
                                         renderer_pipeline_create_info->layout_handle, pipeline_layout);

    VkComputePipelineCreateInfo compute_pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .layout = pipeline_layout,
        .stage = stage_create_info,
    };

    VkPipeline pipeline;
    RV_RETURN_IF_ERROR(context->deps.logger, result,
                       vkCreateComputePipelines(context->device, VK_NULL_HANDLE, 1, &compute_pipeline_create_info, NULL, &pipeline),
                       -1, "Failed to create compute pipeline: %d", result);

    RendererVulkanHandle pipeline_handle = {0};
    RV_RES_RV_HANDLE_ALLOC_OR_RETURN(context->deps.logger, context->pipeline_occupied_a, context->pipeline_generations_a, context->pipelines_a, pipeline, pipeline_handle,
                                     vkDestroyPipeline(context->device, pipeline, NULL));

    *out_pipeline_handle = pipeline_handle.raw;
    return 0;
}


int32_t renderer_vulkan_destroy_shader(RendererContext *context, RendererShaderHandle shader_handle)
{
    assert(context != NULL);

    int32_t ret;

    VkShaderModule shader_module = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->shader_module_generations_a, context->shader_modules_a, shader_handle, shader_module);
    RV_RES_RENDERER_HANDLE_FREE_RETURN_IF_ERROR(context->deps.logger, context->shader_module_occupied_a, context->shader_module_generations_a, context->shader_modules_a, shader_handle);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->active_frame_state.frame->destroy_queue_a, vkDestroyShaderModule, context->device, shader_module, NULL),
                    "Failed to push shader destroy queue: %d", ret);

    return 0;
}

int32_t renderer_vulkan_destroy_pipeline_layout(RendererContext *context, RendererPipelineLayoutHandle pipeline_layout_handle)
{
    assert(context != NULL);

    int32_t ret;

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->pipeline_layout_generations_a, context->pipeline_layouts_a, pipeline_layout_handle, pipeline_layout);
    RV_RES_RENDERER_HANDLE_FREE_RETURN_IF_ERROR(context->deps.logger, context->pipeline_layout_occupied_a, context->pipeline_layout_generations_a, context->pipeline_layouts_a, pipeline_layout_handle);
    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->active_frame_state.frame->destroy_queue_a, vkDestroyPipelineLayout, context->device, pipeline_layout, NULL),
                    "Failed to push pipeline layout to destroy queue: %d", ret);
    return 0;
}

int32_t renderer_vulkan_destroy_graphics_pipeline(RendererContext *context, RendererGraphicsPipelineHandle pipeline_handle)
{
    assert(context != NULL);

    int32_t ret;

    VkPipeline pipeline = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->pipeline_generations_a, context->pipelines_a, pipeline_handle, pipeline);
    RV_RES_RENDERER_HANDLE_FREE_RETURN_IF_ERROR(context->deps.logger, context->pipeline_occupied_a, context->pipeline_generations_a, context->pipelines_a, pipeline_handle);
    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->active_frame_state.frame->destroy_queue_a, vkDestroyPipeline, context->device, pipeline, NULL),
                    "Failed to push graphics pipeline to destroy queue: %d", ret);
    return 0;
}

int32_t renderer_vulkan_destroy_compute_pipeline(RendererContext *context, RendererComputePipelineHandle pipeline_handle)
{
    assert(context != NULL);

    int32_t ret;

    VkPipeline pipeline = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->pipeline_generations_a, context->pipelines_a, pipeline_handle, pipeline);
    RV_RES_RENDERER_HANDLE_FREE_RETURN_IF_ERROR(context->deps.logger, context->pipeline_occupied_a, context->pipeline_generations_a, context->pipelines_a, pipeline_handle);
    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->active_frame_state.frame->destroy_queue_a, vkDestroyPipeline, context->device, pipeline, NULL),
                    "Failed to push compute pipeline to destroy queue: %d", ret);
    return 0;
}