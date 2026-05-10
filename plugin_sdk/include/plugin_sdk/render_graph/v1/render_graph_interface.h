#pragma once

typedef struct RenderGraphContext RenderGraphContext;

typedef struct RenderGraphVtable
{
    void (*dummy)(void);
} RenderGraphVtable;

typedef struct RenderGraphInterface
{
    RenderGraphContext *context;
    RenderGraphVtable *vtable;
} RenderGraphInterface;