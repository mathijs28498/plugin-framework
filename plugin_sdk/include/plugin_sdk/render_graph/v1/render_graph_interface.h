#pragma once

#include <stdint.h>

#include "../../plugin_utils.h"

#pragma pack(push, 8)

typedef struct RenderGraphContext RenderGraphContext;

typedef uint64_t RG_Handle;
typedef struct RG_PassThing
{
    RG_Handle handle;
} RG_PassThing;

typedef struct RG_Pass
{
    uint32_t inputs_len;
    RG_PassThing inputs;
    uint32_t outputs_len;
    RG_PassThing outputs;
} RG_Pass;

typedef struct RenderGraphVtable
{
    int32_t (*register_pass)(RenderGraphContext *context, RG_Pass *pass);
    int32_t (*render)(RenderGraphContext *context);
} RenderGraphVtable;

typedef struct RenderGraphInterface
{
    RenderGraphContext *context;
    RenderGraphVtable *vtable;
} RenderGraphInterface;

#pragma pack(pop)

static inline int32_t render_graph_register_pass(RenderGraphInterface *iface, RG_Pass *pass)
{
    return iface->vtable->register_pass(iface->context, pass);
}

static inline int32_t render_graph_render(RenderGraphInterface *iface)
{
    return iface->vtable->render(iface->context);
}