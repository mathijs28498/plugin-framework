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

typedef struct RendererVtable
{
    int32_t (*start)(RendererContext *context);
    int32_t (*begin_frame)(RendererContext *context, RendererCommandList **out_command_list);
    int32_t (*end_frame)(RendererContext *context);
    void (*on_window_resize)(RendererContext *context, uint32_t width, uint32_t height);

    void (*cmd_begin_render_pass)(RendererContext *context, RendererCommandList *command_list);
    void (*cmd_end_render_pass)(RendererContext *context, RendererCommandList *command_list);
    void (*cmd_bind_pipeline)(RendererContext *context, RendererCommandList *command_list, uint32_t pipeline_handle);
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

static inline void renderer_cmd_begin_render_pass(RendererInterface *iface, RendererCommandList *command_list)
{
    iface->vtable->cmd_begin_render_pass(iface->context, command_list);
}

static inline void renderer_cmd_end_render_pass(RendererInterface *iface, RendererCommandList *command_list)
{
    iface->vtable->cmd_end_render_pass(iface->context, command_list);
}

static inline void renderer_cmd_bind_pipeline(RendererInterface *iface, RendererCommandList *command_list, uint32_t pipeline_handle)
{
    iface->vtable->cmd_bind_pipeline(iface->context, command_list, pipeline_handle);
}

static inline void renderer_cmd_draw(RendererInterface *iface, RendererCommandList *command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
    iface->vtable->cmd_draw(iface->context, command_list, vertex_count, instance_count, first_vertex, first_instance);
}

#pragma pack(pop)
