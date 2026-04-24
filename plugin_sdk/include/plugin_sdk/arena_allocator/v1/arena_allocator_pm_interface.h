#pragma once

#include "arena_allocator_interface.h"

#include <stdint.h>
#include <stddef.h>

struct ArenaAllocatorContext;

#pragma pack(push, 8)

typedef struct ArenaAllocatorPMVtable
{
    ArenaAllocatorVtable public_vtable;

    void (*set_memory_pool)(struct ArenaAllocatorContext *context, uint8_t *memory_pool, const size_t memory_pool_size);
    void (*freeze_permanent_arenas)(struct ArenaAllocatorContext *context);

} ArenaAllocatorPMVtable;

#pragma pack(pop)

inline void arena_allocator_pm_set_memory_pool(ArenaAllocatorInterface *iface, uint8_t *memory_pool, const size_t memory_pool_size)
{
    ((ArenaAllocatorPMVtable *)iface->vtable)->set_memory_pool(iface->context, memory_pool, memory_pool_size);
}

inline void arena_allocator_pm_freeze_permanent_arenas(ArenaAllocatorInterface *iface)
{
    ((ArenaAllocatorPMVtable *)iface->vtable)->freeze_permanent_arenas(iface->context);
}