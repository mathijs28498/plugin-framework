#include "renderer_vulkan_pipeline.h"

#include <plugin_sdk/plugin_utils.h>

#include <assert.h>
#include <stdint.h>
#include <vulkan/vulkan.h>
TODO("Remove this dependency")
#include <stdlib.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_pipeline, LOG_LEVEL_DEBUG)

#include "renderer_vulkan_register.h"

#define MAX_SHADER_STAGES 2

typedef struct RV_PipelineBuilder
{
    VkPipelineShaderStageCreateInfo shader_stage_ci_list[MAX_SHADER_STAGES];
    size_t shader_stage_ci_list_len;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_ci;
    VkPipelineRasterizationStateCreateInfo rasterizer_ci;
    VkPipelineColorBlendAttachmentState color_blend_attachment_ci;
    VkPipelineMultisampleStateCreateInfo multisample_ci;
    VkPipelineDepthStencilStateCreateInfo depth_stencil_ci;
    VkPipelineRenderingCreateInfo rendering_ci;

    VkPipelineLayout pipeline_layout;
    VkFormat color_attachment_format;

} RV_PipelineBuilder;

int32_t rv_pipeline_create_pipeline_builder(RV_PipelineBuilder **out_pipeline_builder)
{
    assert(out_pipeline_builder != NULL);

    TODO("Make this use arena, not calloc");
    RV_PipelineBuilder *pipeline_builder = calloc(1, sizeof(RV_PipelineBuilder));
    if (*out_pipeline_builder == NULL)
    {
        return -1;
    }

    pipeline_builder->input_assembly_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipeline_builder->rasterizer_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipeline_builder->multisample_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipeline_builder->depth_stencil_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipeline_builder->rendering_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

    *out_pipeline_builder = pipeline_builder;

    return 0;
}

int32_t rv_pipeline_builder_build(RendererContext *context, RV_PipelineBuilder *pipeline_builder, VkPipeline *out_pipeline)
{
    assert(context != NULL);
    assert(pipeline_builder != NULL);
    assert(out_pipeline != NULL);

    VkResult result;

    VkPipelineViewportStateCreateInfo viewport_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineColorBlendStateCreateInfo color_blend_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &pipeline_builder->color_blend_attachment_ci,
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pDynamicStates = dynamic_states,
        .dynamicStateCount = ARRAY_SIZE(dynamic_states),
    };

    VkGraphicsPipelineCreateInfo pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &pipeline_builder->rendering_ci,

        .stageCount = (uint32_t)pipeline_builder->shader_stage_ci_list_len,
        .pStages = pipeline_builder->shader_stage_ci_list,
        .pVertexInputState = &vertex_input_create_info,
        .pInputAssemblyState = &pipeline_builder->input_assembly_ci,
        .pViewportState = &viewport_create_info,
        .pRasterizationState = &pipeline_builder->rasterizer_ci,
        .pMultisampleState = &pipeline_builder->multisample_ci,
        .pColorBlendState = &color_blend_create_info,
        .pDepthStencilState = &pipeline_builder->depth_stencil_ci,
        .layout = pipeline_builder->pipeline_layout,

        .pDynamicState = &dynamic_state_create_info,
    };

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkCreateGraphicsPipelines(context->device, NULL, 1, &pipeline_create_info, NULL, out_pipeline),
                       -1, "Unable to create graphics pipeline: %d", result);

    free(pipeline_builder);
    return 0;
}

void rv_pipeline_set_layout(RV_PipelineBuilder *pipeline_builder, VkPipelineLayout pipeline_layout)
{
    assert(pipeline_builder != NULL);
    assert(pipeline_layout != NULL);
    pipeline_builder->pipeline_layout = pipeline_layout;
}

VkPipelineShaderStageCreateInfo rv_pipeline_create_shader_stage_ci(RV_VkShaderStageFlagBits shader_stage, VkShaderModule shader_module)
{
    assert(shader_module != NULL);
    return (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = (VkShaderStageFlagBits)shader_stage,
        .module = shader_module,
        .pName = "main",
    };
}

int32_t rv_pipeline_set_shaders(RV_PipelineBuilder *pipeline_builder, VkShaderModule vertex_shader, VkShaderModule fragment_shader)
{
    assert(pipeline_builder != NULL);
    assert(vertex_shader != NULL);
    assert(fragment_shader != NULL);

    if (ARRAY_SIZE(pipeline_builder->shader_stage_ci_list) < pipeline_builder->shader_stage_ci_list_len + 2)
    {
        return -1;
    }

    pipeline_builder->shader_stage_ci_list[pipeline_builder->shader_stage_ci_list_len] =
        rv_pipeline_create_shader_stage_ci(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader);
    pipeline_builder->shader_stage_ci_list[pipeline_builder->shader_stage_ci_list_len + 1] =
        rv_pipeline_create_shader_stage_ci(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader);

    pipeline_builder->shader_stage_ci_list_len += 2;

    return 0;
}

void rv_pipeline_set_input_topology(RV_PipelineBuilder *pipeline_builder, RV_VkPrimitiveTopology topology)
{
    assert(pipeline_builder != NULL);
    pipeline_builder->input_assembly_ci.topology = (VkPrimitiveTopology)topology;
    pipeline_builder->input_assembly_ci.primitiveRestartEnable = VK_FALSE;
}

void rv_pipeline_set_polygon_mode(RV_PipelineBuilder *pipeline_builder, RV_VkPolygonMode polygon_mode)
{
    assert(pipeline_builder != NULL);
    pipeline_builder->rasterizer_ci.polygonMode = (VkPolygonMode)polygon_mode;
    pipeline_builder->rasterizer_ci.lineWidth = 1.0;
}

void rv_pipeline_set_cull_mode(RV_PipelineBuilder *pipeline_builder, RV_VkCullModeFlags cull_mode_flags, RV_VkFrontFace front_face)
{
    assert(pipeline_builder != NULL);
    pipeline_builder->rasterizer_ci.cullMode = (VkCullModeFlags)cull_mode_flags;
    pipeline_builder->rasterizer_ci.frontFace = (VkFrontFace)front_face;
}

void rv_pipeline_set_multisampling_none(RV_PipelineBuilder *pipeline_builder)
{
    assert(pipeline_builder != NULL);
    pipeline_builder->multisample_ci.sampleShadingEnable = VK_FALSE;

    // multisampling defaulted to no multisampling (1 sample per pixel)
    pipeline_builder->multisample_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipeline_builder->multisample_ci.minSampleShading = 1.0f;
    pipeline_builder->multisample_ci.pSampleMask = NULL;

    // no alpha to coverage either
    pipeline_builder->multisample_ci.alphaToCoverageEnable = VK_FALSE;
    pipeline_builder->multisample_ci.alphaToOneEnable = VK_FALSE;
}

void rv_pipeline_disable_blending(RV_PipelineBuilder *pipeline_builder)
{
    assert(pipeline_builder != NULL);
    pipeline_builder->color_blend_attachment_ci.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipeline_builder->color_blend_attachment_ci.blendEnable = VK_FALSE;
}

void rv_pipeline_set_color_attachment_format(RV_PipelineBuilder *pipeline_builder, RV_VkFormat format)
{
    assert(pipeline_builder != NULL);
    pipeline_builder->color_attachment_format = format;
    pipeline_builder->rendering_ci.colorAttachmentCount = 1;
    pipeline_builder->rendering_ci.pColorAttachmentFormats = (VkFormat *)&pipeline_builder->color_attachment_format;
}

void rv_pipeline_set_depth_format(RV_PipelineBuilder *pipeline_builder, RV_VkFormat format)
{
    assert(pipeline_builder != NULL);
    pipeline_builder->rendering_ci.depthAttachmentFormat = (VkFormat)format;
}

void rv_pipeline_disable_depthtest(RV_PipelineBuilder *pipeline_builder)
{
    assert(pipeline_builder != NULL);
    pipeline_builder->depth_stencil_ci.depthTestEnable = VK_FALSE;
    pipeline_builder->depth_stencil_ci.depthWriteEnable = VK_FALSE;
    pipeline_builder->depth_stencil_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
    pipeline_builder->depth_stencil_ci.depthBoundsTestEnable = VK_FALSE;
    pipeline_builder->depth_stencil_ci.stencilTestEnable = VK_FALSE;
    pipeline_builder->depth_stencil_ci.minDepthBounds = 0.f;
    pipeline_builder->depth_stencil_ci.maxDepthBounds = 1.f;
}
