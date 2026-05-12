#pragma once

#include "plugin_dependencies.h"

#include <stdint.h>

#pragma pack(push, 8)
typedef uint64_t RendererPipelineLayoutHandle;
typedef uint64_t RendererPipelineHandle;
typedef uint64_t RendererResourceSetLayoutHandle;

typedef struct DrawContext
{
    PluginDependencies deps;
    RendererResourceSetLayoutHandle draw_image_resource_set_layout_handle;
    RendererPipelineLayoutHandle gradient_pipeline_layout_handle;
    RendererPipelineHandle background_pipeline_handle;

} DrawContext;

#pragma pack(pop)