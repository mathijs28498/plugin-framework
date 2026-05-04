#include "renderer_vulkan_pipeline.h"

#include <plugin_sdk/plugin_utils.h>

#include <assert.h>
#include <stdint.h>
#include <vulkan/vulkan.h>
TODO("Remove this dependency")
#include <stdlib.h>
#include <stdbool.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_pipeline, LOG_LEVEL_DEBUG)
#include <plugin_sdk/renderer/v1/renderer_interface.h>

#include "shader_colored_triangle_vertex.h"
#include "shader_colored_triangle_fragment.h"

#include "renderer_vulkan_register.h"
#include "renderer_vulkan.h"

#define MAX_SHADER_STAGES 2

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
    RV_RES_HANDLE_ALLOC_RETURN_IF_ERROR(context->deps.logger, context->shader_modules, context->shader_module_generations, shader_module, shader_handle,
                                                     vkDestroyShaderModule(context->device, shader_module, NULL));
    *out_shader_handle = shader_handle.raw;
    return 0;
}

int32_t renderer_vulkan_destroy_shader(RendererContext *context, RendererShaderHandle shader_handle)
{
    assert(context != NULL);

    RendererVulkanHandle vulkan_shader_handle = {.raw = shader_handle};

    VkShaderModule shader_module;
    RV_RES_HANDLE_GET_OR_RETURN(context->deps.logger, context->shader_modules, context->shader_module_generations, vulkan_shader_handle, shader_module);
    RV_RES_HANDLE_FREE_RETURN_IF_ERROR(context->deps.logger, context->shader_modules, context->shader_module_generations, vulkan_shader_handle);

    vkDestroyShaderModule(context->device, shader_module, NULL);

    return 0;
}

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

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkCreateGraphicsPipelines(context->device, NULL, 1, &pipeline_create_info, NULL, out_pipeline),
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

void rv_pipeline_enable_blending_additive(RV_PipelineBuilder *pipeline_builder)
{
    assert(pipeline_builder != NULL);
    pipeline_builder->color_blend_attachment_ci.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipeline_builder->color_blend_attachment_ci.blendEnable = VK_TRUE;
    pipeline_builder->color_blend_attachment_ci.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    pipeline_builder->color_blend_attachment_ci.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    pipeline_builder->color_blend_attachment_ci.colorBlendOp = VK_BLEND_OP_ADD;
    pipeline_builder->color_blend_attachment_ci.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    pipeline_builder->color_blend_attachment_ci.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipeline_builder->color_blend_attachment_ci.alphaBlendOp = VK_BLEND_OP_ADD;
}

void rv_pipeline_enable_blending_alphablend(RV_PipelineBuilder *pipeline_builder)
{
    assert(pipeline_builder != NULL);
    pipeline_builder->color_blend_attachment_ci.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipeline_builder->color_blend_attachment_ci.blendEnable = VK_TRUE;
    pipeline_builder->color_blend_attachment_ci.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    pipeline_builder->color_blend_attachment_ci.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipeline_builder->color_blend_attachment_ci.colorBlendOp = VK_BLEND_OP_ADD;
    pipeline_builder->color_blend_attachment_ci.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    pipeline_builder->color_blend_attachment_ci.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipeline_builder->color_blend_attachment_ci.alphaBlendOp = VK_BLEND_OP_ADD;
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

int32_t renderer_vulkan_create_pipeline_layout(RendererContext *context, const RendererPipelineLayoutCreateInfo *renderer_pipeline_layout_create_info, RendererPipelineLayoutHandle *out_pipeline_layout_handle)
{
    assert(context != NULL);
    assert(renderer_pipeline_layout_create_info != NULL);
    assert(out_pipeline_layout_handle != NULL);

    VkResult result;
    int32_t ret;

    TODO("Allow for push constants coming from the arguments")
    VkPushConstantRange push_constant_range = {
        .offset = 0,
        .size = renderer_pipeline_layout_create_info->push_constant_size,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    };

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pSetLayouts = &context->draw_image_descriptor_set_layout,
        .setLayoutCount = 1,
        .pPushConstantRanges = &push_constant_range,
        .pushConstantRangeCount = 1,
    };

    VkPipelineLayout pipeline_layout;
    RV_RETURN_IF_ERROR(context->deps.logger, result, vkCreatePipelineLayout(context->device, &pipeline_layout_create_info, NULL, &pipeline_layout),
                       -1, "Failed to create pipeline layout: %d", result);

    RendererVulkanHandle rv_pipeline_layout_handle = {0};
    RV_RES_HANDLE_ALLOC_RETURN_IF_ERROR(context->deps.logger, context->pipeline_layouts, context->pipeline_layout_generations, pipeline_layout, rv_pipeline_layout_handle,
                                                     vkDestroyPipelineLayout(context->device, pipeline_layout, NULL));

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->main_destroy_queue, vkDestroyPipelineLayout, context->device, pipeline_layout, NULL),
                    "Failed to push gradient pipeline layout destroy data to destroy queue: %d", ret);

    *out_pipeline_layout_handle = rv_pipeline_layout_handle.raw;

    return 0;
}

int32_t renderer_vulkan_create_graphics_pipeline(RendererContext *context, const RendererGraphicsPipelineCreateInfo *renderer_pipeline_create_info, RendererGraphicsPipelineHandle *out_pipeline_handle)
{
    assert(context != NULL);
    assert(renderer_pipeline_create_info != NULL);
    assert(out_pipeline_handle != NULL);
    return 0;
}

int32_t renderer_vulkan_create_compute_pipeline(RendererContext *context, const RendererComputePipelineCreateInfo *renderer_pipeline_create_info, RendererComputePipelineHandle *out_pipeline_handle)
{
    assert(context != NULL);
    assert(renderer_pipeline_create_info != NULL);
    assert(out_pipeline_handle != NULL);

    VkResult result;
    int32_t ret;

    // VkPushConstantRange compute_push_constant_range = {
    //     .offset = 0,
    //     .size = sizeof(ComputePushConstants),
    //     .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    // };

    // VkPipelineLayoutCreateInfo compute_pipeline_layout_create_info = {
    //     .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    //     .pSetLayouts = &context->draw_image_descriptor_set_layout,
    //     .setLayoutCount = 1,
    //     .pPushConstantRanges = &compute_push_constant_range,
    //     .pushConstantRangeCount = 1,
    // };

    // VK_RETURN_IF_ERROR(context->deps.logger, result, vkCreatePipelineLayout(context->device, &compute_pipeline_layout_create_info, NULL, &context->gradient_pipeline_layout),
    //                    -1, "Failed to create gradient pipeline layout: %d", result);

    // RETURN_IF_ERROR(context->deps.logger, ret,
    //                 RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->main_destroy_queue, vkDestroyPipelineLayout, context->device, context->gradient_pipeline_layout, NULL),
    //                 "Failed to push gradient pipeline layout destroy data to destroy queue: %d", ret);

    // TODO("REMOVE THIS")
    // VkShaderModule compute_shader_module = NULL;
    // RETURN_IF_ERROR(context->deps.logger, ret, renderer_vulkan_create_shader(context, GRADIENT_COMPUTE_SHADER_U32_CODE, GRADIENT_COMPUTE_SHADER_BYTES_LEN, &compute_shader_module),
    //                 "Failed to load shader module: %d", ret);
    // RendererShaderHandle compute_shader_handle;
    // RETURN_IF_ERROR(context->deps.logger, ret,
    //                 renderer_vulkan_create_shader(context, GRADIENT_COMPUTE_SHADER_U32_CODE, GRADIENT_COMPUTE_SHADER_BYTES_LEN, &compute_shader_handle),
    //                 "Failed to create shader: %d", ret);

    VkShaderModule compute_shader_module;
    RendererVulkanHandle rv_compute_shader_handle = {.raw = renderer_pipeline_create_info->compute_shader_handle};
    RV_RES_HANDLE_GET_OR_RETURN(context->deps.logger, context->shader_modules, context->shader_module_generations, rv_compute_shader_handle, compute_shader_module);

    VkPipelineShaderStageCreateInfo stage_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = compute_shader_module,
        .pName = renderer_pipeline_create_info->compute_shader_entry_point,
    };

    VkPipelineLayout pipeline_layout;
    RendererVulkanHandle rv_pipeline_layout_handle = {.raw = renderer_pipeline_create_info->layout_handle};
    RV_RES_HANDLE_GET_OR_RETURN(context->deps.logger, context->pipeline_layouts, context->pipeline_layout_generations, rv_pipeline_layout_handle, pipeline_layout);

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
    RV_RES_HANDLE_ALLOC_RETURN_IF_ERROR(context->deps.logger, context->pipelines, context->pipeline_generations, pipeline, pipeline_handle,
                                                     vkDestroyPipeline(context->device, pipeline, NULL));

    TODO("Make owner responsible for destruction");
    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->main_destroy_queue, vkDestroyPipeline, context->device, pipeline, NULL),
                    "Failed to push gradient pipeline destroy data to destroy queue: %d", ret);

    // RETURN_IF_ERROR(context->deps.logger, ret, renderer_vulkan_destroy_shader(context, compute_shader_handle),
    // "Failed to destroy shader: %d", ret);
    *out_pipeline_handle = pipeline_handle.raw;

    return 0;
}

int32_t create_triangle_pipeline(RendererContext *context)
{
    assert(context != NULL);

    int32_t ret;
    VkResult result;

    TODO("REMVOE THIS");
    VkShaderModule vertex_shader_module = NULL, fragment_shader_module = NULL;
    // RETURN_IF_ERROR(context->deps.logger, ret, renderer_vulkan_create_shader(context, COLORED_TRIANGLE_VERTEX_SHADER_U32_CODE, COLORED_TRIANGLE_VERTEX_SHADER_BYTES_LEN, &vertex_shader_module),
    //                 "Failed to load vertex shader module: %d", ret);
    // RETURN_IF_ERROR(context->deps.logger, ret, renderer_vulkan_create_shader(context, COLORED_TRIANGLE_FRAGMENT_SHADER_U32_CODE, COLORED_TRIANGLE_FRAGMENT_SHADER_BYTES_LEN, &fragment_shader_module),
    //                 "Failed to load fragment shader module: %d", ret);

    VkPipelineLayoutCreateInfo graphics_pipeline_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };

    VkPipelineLayout graphics_pipeline_layout;
    RV_RETURN_IF_ERROR(context->deps.logger, result, vkCreatePipelineLayout(context->device, &graphics_pipeline_layout_create_info, NULL, &graphics_pipeline_layout),
                       -1, "Failed to create graphics pipeline layout: %d", result);

    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->main_destroy_queue, vkDestroyPipelineLayout, context->device, graphics_pipeline_layout, NULL);

    RV_PipelineBuilder *pipeline_builder;
    RETURN_IF_ERROR(context->deps.logger, ret, rv_pipeline_create_pipeline_builder(&pipeline_builder),
                    "Failed to create pipeline builder: %d", ret);

    rv_pipeline_set_layout(pipeline_builder, graphics_pipeline_layout);
    RETURN_IF_ERROR(context->deps.logger, ret, rv_pipeline_set_shaders(pipeline_builder, vertex_shader_module, fragment_shader_module),
                    "Failed to set shaders: %d", ret);
    rv_pipeline_set_input_topology(pipeline_builder, RV_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    rv_pipeline_set_polygon_mode(pipeline_builder, RV_POLYGON_MODE_FILL);
    rv_pipeline_set_cull_mode(pipeline_builder, RV_CULL_MODE_NONE, RV_FRONT_FACE_CLOCKWISE);
    rv_pipeline_set_multisampling_none(pipeline_builder);
    rv_pipeline_disable_blending(pipeline_builder);
    // rv_pipeline_enable_blending_additive(pipeline_builder);
    // rv_pipeline_enable_blending_alphablend(pipeline_builder);
    rv_pipeline_disable_depthtest(pipeline_builder);

    rv_pipeline_set_color_attachment_format(pipeline_builder, context->draw_image.image_format);
    rv_pipeline_set_depth_format(pipeline_builder, VK_FORMAT_UNDEFINED);

    RETURN_IF_ERROR(context->deps.logger, ret, rv_pipeline_builder_build(context, pipeline_builder, &context->triangle_pipeline),
                    "Failed to build graphics pipeline: %d", ret);

    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->main_destroy_queue, vkDestroyPipeline, context->device, context->triangle_pipeline, NULL);

    vkDestroyShaderModule(context->device, fragment_shader_module, NULL);
    vkDestroyShaderModule(context->device, vertex_shader_module, NULL);

    return 0;
}