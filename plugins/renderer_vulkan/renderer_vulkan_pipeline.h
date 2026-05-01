#pragma once

#include <stdint.h>

#include "renderer_vulkan_utils.h"

// Forward declare the context
struct RendererContext;

CREATE_VK_HANDLE_DEFINITION(VkPipeline);
CREATE_VK_HANDLE_DEFINITION(VkShaderModule);
CREATE_VK_HANDLE_DEFINITION(VkPipelineLayout);

typedef struct RV_PipelineBuilder RV_PipelineBuilder;

typedef enum RV_VkPrimitiveTopology
{
    RV_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
    RV_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
    RV_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
    RV_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
    RV_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
    RV_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN = 5,
} RV_VkPrimitiveTopology;

typedef enum RV_VkPolygonMode
{
    RV_POLYGON_MODE_FILL = 0,
    RV_POLYGON_MODE_LINE = 1,
    RV_POLYGON_MODE_POINT = 2,
} RV_VkPolygonMode;

typedef enum RV_CullModeFlagBits
{
    RV_CULL_MODE_NONE = 0,
    RV_CULL_MODE_FRONT_BIT = 0x00000001,
    RV_CULL_MODE_BACK_BIT = 0x00000002,
    RV_CULL_MODE_FRONT_AND_BACK = 0x00000003,
} RV_CullModeFlagBits;

typedef enum RV_VkFrontFace
{
    RV_FRONT_FACE_COUNTER_CLOCKWISE = 0,
    RV_FRONT_FACE_CLOCKWISE = 1,
} RV_VkFrontFace;

typedef uint32_t RV_VkCullModeFlags;
typedef uint32_t RV_VkFormat;
typedef uint32_t RV_VkShaderStageFlagBits;

int32_t rv_pipeline_create_pipeline_builder(RV_PipelineBuilder **out_pipeline_builder);
int32_t rv_pipeline_builder_build(struct RendererContext *context, RV_PipelineBuilder *pipeline_builder, VkPipeline *out_pipeline);

void rv_pipeline_set_layout(RV_PipelineBuilder *pipeline_builder, VkPipelineLayout pipeline_layout);
int32_t rv_pipeline_set_shaders(RV_PipelineBuilder *pipeline_builder, VkShaderModule vertex_shader, VkShaderModule fragment_shader);
void rv_pipeline_set_input_topology(RV_PipelineBuilder *pipeline_builder, RV_VkPrimitiveTopology topology);
void rv_pipeline_set_polygon_mode(RV_PipelineBuilder *pipeline_builder, RV_VkPolygonMode polygon_mode);
void rv_pipeline_set_cull_mode(RV_PipelineBuilder *pipeline_builder, RV_VkCullModeFlags cull_mode_flags, RV_VkFrontFace front_face);
void rv_pipeline_set_multisampling_none(RV_PipelineBuilder *pipeline_builder);
void rv_pipeline_disable_blending(RV_PipelineBuilder *pipeline_builder);
void rv_pipeline_enable_blending_additive(RV_PipelineBuilder *pipeline_builder);
void rv_pipeline_enable_blending_alphablend(RV_PipelineBuilder *pipeline_builder);
void rv_pipeline_set_color_attachment_format(RV_PipelineBuilder *pipeline_builder, RV_VkFormat format);
void rv_pipeline_set_depth_format(RV_PipelineBuilder *pipeline_builder, RV_VkFormat format);
void rv_pipeline_disable_depthtest(RV_PipelineBuilder *pipeline_builder);

struct VkPipelineShaderStageCreateInfo rv_pipeline_create_shader_stage_ci(RV_VkShaderStageFlagBits shader_stage, VkShaderModule shader_module);