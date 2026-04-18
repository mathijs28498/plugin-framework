#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "plugin_utils.h"

#pragma pack(push, 8)

struct RendererContext;

TODO("Figure out what variables config needs to have")
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
    // int32_t (*create_swapchain)(struct RendererContext *context, RendererWindowConfig *window_config);
    int32_t (*start)(struct RendererContext *context);
    int32_t (*render)(struct RendererContext *context);
    void (*on_window_resize)(struct RendererContext *context, uint32_t width, uint32_t height);
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

static inline int32_t renderer_render(RendererInterface *iface)
{
    return iface->vtable->render(iface->context);
}

static inline void renderer_on_window_resize(RendererInterface *iface, uint32_t width, uint32_t height)
{
    iface->vtable->on_window_resize(iface->context, width, height);
}

#pragma pack(pop)
