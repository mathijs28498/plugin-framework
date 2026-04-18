#pragma once

#include <stdint.h>

#pragma pack(push, 8)

struct DrawContext;

typedef struct DrawVtable
{
    int32_t (*start)(struct DrawContext *context);
    int32_t (*present)(struct DrawContext *context);
    void (*on_window_resize)(struct DrawContext *context, uint32_t width, uint32_t height);
} DrawVtable;

typedef struct DrawInterface
{
    struct DrawContext *context;
    DrawVtable *vtable;

} DrawInterface;

#pragma pack(pop)

static inline int32_t draw_start(DrawInterface *iface)
{
    return iface->vtable->start(iface->context);
}

static inline int32_t draw_present(DrawInterface *iface)
{
    return iface->vtable->present(iface->context);
}

static inline void draw_on_window_resize(DrawInterface *iface, uint32_t width, uint32_t height)
{
    iface->vtable->on_window_resize(iface->context, width, height);
}