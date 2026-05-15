#pragma once

#include "plugin_dependencies.h"

#include <stdint.h>

#include <plugin_sdk/renderer/v1/renderer_types.h>

#pragma pack(push, 8)


typedef struct DrawContext
{
    PluginDependencies deps;
    RendererResourceSetLayoutHandle draw_image_resource_set_layout_handle;
    RendererPipelineLayoutHandle gradient_pipeline_layout_handle;
    RendererComputePipelineHandle background_pipeline_handle;

    RendererImageHandle draw_image_handle;
    RendererExtent2D draw_extent;
} DrawContext;

#pragma pack(pop)