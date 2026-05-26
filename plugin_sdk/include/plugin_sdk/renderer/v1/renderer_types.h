#pragma once

#include <stdint.h>

typedef uint64_t RendererShaderHandle;
typedef uint64_t RendererResourceSetLayoutHandle;
typedef uint64_t RendererResourceSetHandle;
typedef uint64_t RendererPipelineLayoutHandle;
typedef uint64_t RendererGraphicsPipelineHandle;
typedef uint64_t RendererComputePipelineHandle;
typedef uint64_t RendererCommandListHandle;
typedef uint64_t RendererImageHandle;
typedef uint64_t RendererSamplerHandle;
typedef uint64_t RendererBufferHandle;
typedef uint64_t RendererBufferHandle;
typedef uint64_t RendererBufferDeviceAddress;

// TODO: Figure out how to do this properly
typedef uint64_t RendererVertexBufferAddress;

typedef struct RendererExtent3D
{
    uint32_t width;
    uint32_t height;
    uint32_t depth;
} RendererExtent3D;

typedef struct RendererExtent2D
{
    uint32_t width;
    uint32_t height;
} RendererExtent2D;

typedef struct RendererCommandList RendererCommandList;

typedef int32_t (*ImmediateExecute_Fn)(RendererCommandList *command_list, void *user_data);