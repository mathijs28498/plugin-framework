#pragma once

#include <stdint.h>

#pragma pack(push, 8)

struct DrawContext;

typedef struct DrawVtable
{
    int32_t (*present)(struct DrawContext *context);
} DrawVtable;

typedef struct DrawInterface
{
    struct DrawContext *context;
    DrawVtable *vtable;

} DrawInterface;

#pragma pack(pop)
static inline int32_t draw_present(DrawInterface *interface)
{
    return interface->vtable->present(interface->context);
}
