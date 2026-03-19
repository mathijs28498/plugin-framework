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
    int32_t (*init)(struct RendererContext *context);
} RendererVtable;

typedef struct RendererInterface
{
    struct RendererContext *context;
    RendererVtable *vtable;
} RendererInterface;

static inline int32_t renderer_init(RendererInterface *iface)
{
    return iface->vtable->init(iface->context);
}

#pragma pack(pop)
