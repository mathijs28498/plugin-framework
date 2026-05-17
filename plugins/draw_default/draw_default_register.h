#pragma once

#include "plugin_dependencies.h"

#include <stdint.h>

#include <cglm/types.h>

#include <plugin_sdk/renderer/v1/renderer_types.h>

#pragma pack(push, 8)

typedef struct BackgroundPushConstants
{
    vec4 top_left;
    vec4 top_right;
    vec4 bottom_left;
    vec4 bottom_right;
} BackgroundPushConstants;

typedef struct DrawContext
{
    PluginDependencies deps;
    RendererResourceSetLayoutHandle draw_image_resource_set_layout_handle;

    RendererPipelineLayoutHandle background_pipeline_layout_handle;
    RendererComputePipelineHandle background_pipeline_handle;

    RendererPipelineLayoutHandle triangle_pipeline_layout_handle;
    RendererGraphicsPipelineHandle triangle_pipeline_handle;

    RendererImageHandle draw_image_handle;
    RendererExtent2D draw_extent;
} DrawContext;

#pragma pack(pop)