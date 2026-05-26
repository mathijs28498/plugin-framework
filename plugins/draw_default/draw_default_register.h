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

typedef struct GPUDrawPushConstants
{
    mat4 world_matrix;
    RendererVertexBufferAddress vertex_buffer_address;
} GPUDrawPushConstants;

typedef struct Vertex
{
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
} Vertex;

typedef struct GPUMeshBuffers
{
    RendererBufferHandle vertex_buffer_handle;
    RendererBufferDeviceAddress vertex_buffer_address;

    RendererBufferHandle index_buffer_handle;
} GPUMeshBuffers;

typedef struct DrawContext
{
    PluginDependencies deps;
    RendererResourceSetLayoutHandle draw_image_resource_set_layout_handle;

    RendererPipelineLayoutHandle background_pipeline_layout_handle;
    RendererComputePipelineHandle background_pipeline_handle;

    RendererPipelineLayoutHandle triangle_pipeline_layout_handle;
    RendererGraphicsPipelineHandle triangle_pipeline_handle;

    RendererPipelineLayoutHandle triangle_mesh_pipeline_layout_handle;
    RendererGraphicsPipelineHandle triangle_mesh_pipeline_handle;

    RendererImageHandle draw_image_handle;
    RendererExtent2D draw_extent;

    GPUMeshBuffers rect_mesh_buffers;
} DrawContext;

#pragma pack(pop)