#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../../plugin_utils.h"

#pragma pack(push, 8)

typedef struct RendererContext RendererContext;
typedef struct RendererCommandList RendererCommandList;

typedef struct RendererWindowConfig
{
    void *window_handle;
    void *display_handle;

    uint32_t width;
    uint32_t height;
    bool enable_vsync;
} RendererWindowConfig;

typedef uint64_t RendererShaderHandle;
typedef uint64_t RendererDescriptorSetHandle;
typedef uint64_t RendererPipelineLayoutHandle;
typedef uint64_t RendererGraphicsPipelineHandle;
typedef uint64_t RendererComputePipelineHandle;
typedef uint64_t RendererCommandListHandle;

typedef enum RendererShaderStage
{
    RENDERER_SHADER_STAGE_VERTEX,
    RENDERER_SHADER_STAGE_FRAGMENT,
    RENDERER_SHADER_STAGE_COMPUTE,
} RendererShaderStage;

typedef enum RendererPipelineType
{
    RENDERER_PIPELINE_TYPE_GRAPHICS,
    RENDERER_PIPELINE_TYPE_COMPUTE,
} RendererPipelineType;

TODO("Allow for multiple push constants")
TODO("Make this create info more complete with handles to descriptor sets")
typedef struct RendererPipelineLayoutCreateInfo
{
    uint32_t push_constant_size;
} RendererPipelineLayoutCreateInfo;

typedef struct RendererGraphicsPipelineCreateInfo
{
    RendererShaderHandle vertex_shader_handle;
    const char *vertex_shader_entry_point;
    RendererShaderHandle fragment_shader_handle;
    const char *fragment_shader_entry_point;

    RendererPipelineLayoutHandle layout_handle;
} RendererGraphicsPipelineCreateInfo;

typedef struct RendererComputePipelineCreateInfo
{
    RendererShaderHandle compute_shader_handle;
    const char *compute_shader_entry_point;

    RendererPipelineLayoutHandle layout_handle;
} RendererComputePipelineCreateInfo;

typedef struct RendererVtable
{
    int32_t (*start)(RendererContext *context);
    int32_t (*begin_frame)(RendererContext *context, RendererCommandList **out_command_list);
    int32_t (*end_frame)(RendererContext *context);
    void (*on_window_resize)(RendererContext *context, uint32_t width, uint32_t height);

    int32_t (*create_shader)(RendererContext *context, const uint32_t *shader_code_u32, size_t shader_code_bytes_len, RendererShaderHandle *out_shader_handle);
    int32_t (*destroy_shader)(RendererContext *context, RendererShaderHandle shader_handle);

    int32_t (*create_pipeline_layout)(RendererContext *context, const RendererPipelineLayoutCreateInfo *renderer_pipeline_layout_create_info, RendererPipelineLayoutHandle *out_pipeline_layout_handle);

    int32_t (*create_graphics_pipeline)(RendererContext *context, const RendererGraphicsPipelineCreateInfo *pipeline_create_info, RendererGraphicsPipelineHandle *out_pipeline_handle);
    int32_t (*create_compute_pipeline)(RendererContext *context, const RendererComputePipelineCreateInfo *pipeline_create_info, RendererComputePipelineHandle *out_pipeline_handle);

    void (*cmd_begin_render_pass)(RendererContext *context, RendererCommandList *command_list);
    void (*cmd_end_render_pass)(RendererContext *context, RendererCommandList *command_list);
    void (*cmd_bind_graphics_pipeline)(RendererContext *context, RendererCommandList *command_list, RendererGraphicsPipelineHandle pipeline_handle);
    void (*cmd_bind_compute_pipeline)(RendererContext *context, RendererCommandList *command_list, RendererComputePipelineHandle pipeline_handle);
    void (*cmd_draw)(RendererContext *context, RendererCommandList *command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
} RendererVtable;

typedef struct RendererInterface
{
    struct RendererContext *context;
    RendererVtable *vtable;
} RendererInterface;

static inline int32_t renderer_start(RendererInterface *iface)
{
    return iface->vtable->start(iface->context);
}

static inline int32_t renderer_begin_frame(RendererInterface *iface, RendererCommandList **out_command_list)
{
    return iface->vtable->begin_frame(iface->context, out_command_list);
}

static inline int32_t renderer_end_frame(RendererInterface *iface)
{
    return iface->vtable->end_frame(iface->context);
}

static inline void renderer_on_window_resize(RendererInterface *iface, uint32_t width, uint32_t height)
{
    iface->vtable->on_window_resize(iface->context, width, height);
}

static inline int32_t renderer_create_shader(RendererInterface *iface, const uint32_t *shader_code_u32, size_t shader_code_bytes_len, RendererShaderHandle *out_shader_handle)
{
    return iface->vtable->create_shader(iface->context, shader_code_u32, shader_code_bytes_len, out_shader_handle);
}

static inline int32_t renderer_destroy_shader(RendererInterface *iface, RendererShaderHandle shader_handle)
{
    return iface->vtable->destroy_shader(iface->context, shader_handle);
}

static inline int32_t renderer_create_pipeline_layout(RendererInterface *iface, const RendererPipelineLayoutCreateInfo *renderer_pipeline_layout_create_info, RendererPipelineLayoutHandle *out_pipeline_layout_handle)
{
    return iface->vtable->create_pipeline_layout(iface->context, renderer_pipeline_layout_create_info, out_pipeline_layout_handle);
}

static inline int32_t renderer_create_graphics_pipeline(RendererInterface *iface, const RendererGraphicsPipelineCreateInfo *pipeline_create_info, RendererGraphicsPipelineHandle *out_pipeline_handle)
{
    return iface->vtable->create_graphics_pipeline(iface->context, pipeline_create_info, out_pipeline_handle);
}

static inline int32_t renderer_create_compute_pipeline(RendererInterface *iface, const RendererComputePipelineCreateInfo *pipeline_create_info, RendererComputePipelineHandle *out_pipeline_handle)
{
    return iface->vtable->create_compute_pipeline(iface->context, pipeline_create_info, out_pipeline_handle);
}

static inline void renderer_cmd_begin_render_pass(RendererInterface *iface, RendererCommandList *command_list)
{
    iface->vtable->cmd_begin_render_pass(iface->context, command_list);
}

static inline void renderer_cmd_end_render_pass(RendererInterface *iface, RendererCommandList *command_list)
{
    iface->vtable->cmd_end_render_pass(iface->context, command_list);
}

static inline void renderer_cmd_bind_graphics_pipeline(RendererInterface *iface, RendererCommandList *command_list, RendererGraphicsPipelineHandle pipeline_handle)
{
    iface->vtable->cmd_bind_graphics_pipeline(iface->context, command_list, pipeline_handle);
}

static inline void renderer_cmd_bind_compute_pipeline(RendererInterface *iface, RendererCommandList *command_list, RendererComputePipelineHandle pipeline_handle)
{
    iface->vtable->cmd_bind_compute_pipeline(iface->context, command_list, pipeline_handle);
}

static inline void renderer_cmd_draw(RendererInterface *iface, RendererCommandList *command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
    iface->vtable->cmd_draw(iface->context, command_list, vertex_count, instance_count, first_vertex, first_instance);
}

#pragma pack(pop)
