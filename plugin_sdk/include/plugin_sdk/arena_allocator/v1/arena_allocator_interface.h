#pragma once

#include <stdint.h>

struct ArenaAllocatorContext;

#pragma pack(push, 8)

typedef struct ArenaAllocatorVtable
{
    int32_t (*get_arena)(struct ArenaAllocatorContext *context, uint64_t size, uint8_t ** out_arena);
} ArenaAllocatorVtable;

typedef struct ArenaAllocatorInterface
{
    struct ArenaAllocatorContext *context;
    const ArenaAllocatorVtable *vtable;
} ArenaAllocatorInterface;

#pragma pack(pop)

inline int32_t arena_allocator_get_arena(ArenaAllocatorInterface *iface, uint64_t size, uint8_t ** out_arena)
{
    return iface->vtable->get_arena(iface->context, size, out_arena);
}