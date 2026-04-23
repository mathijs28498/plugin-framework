#pragma once

#include "arena_allocator_interface.h"

#include <stdint.h>

struct ArenaAllocatorContext;

#pragma pack(push, 8)

typedef struct ArenaAllocatorPMVtable
{
    ArenaAllocatorVtable public_vtable;

    void (*set_memory_pool)(struct ArenaAllocatorContext *context, uint8_t *memory_pool, const uint64_t memory_pool_size);

} ArenaAllocatorPMVtable;

#pragma pack(pop)

inline void arena_allocator_pm_set_memory_pool(ArenaAllocatorInterface *iface, uint8_t *memory_pool, const uint64_t memory_pool_size)
{
    ((ArenaAllocatorPMVtable *)iface->vtable)->set_memory_pool(iface->context, memory_pool, memory_pool_size);
}