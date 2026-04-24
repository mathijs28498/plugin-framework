#pragma once

#include <stdint.h>
#include <stddef.h>

struct ArenaAllocatorContext;

#pragma pack(push, 8)

typedef struct ArenaAllocatorVtable
{
    int32_t (*get_arena)(struct ArenaAllocatorContext *context, size_t size, void **out_arena);
    int32_t (*register_pools)(struct ArenaAllocatorContext *context, uint32_t *id);
} ArenaAllocatorVtable;

typedef struct ArenaAllocatorInterface
{
    struct ArenaAllocatorContext *context;
    const ArenaAllocatorVtable *vtable;
} ArenaAllocatorInterface;

#pragma pack(pop)

inline int32_t arena_allocator_get_arena(ArenaAllocatorInterface *iface, size_t size, void **out_arena)
{
    return iface->vtable->get_arena(iface->context, size, out_arena);
}

inline int32_t arena_allocator_register_pools(ArenaAllocatorInterface *iface, uint32_t *id)
{
    return iface->vtable->register_pools(iface->context, id);
}