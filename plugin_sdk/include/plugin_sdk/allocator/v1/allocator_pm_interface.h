#pragma once

#include "allocator_interface.h"

#include <stdint.h>
#include <stddef.h>

struct AllocatorContext;

#pragma pack(push, 8)

typedef struct AllocatorPMVtable
{
    AllocatorVtable public_vtable;

    void (*set_memory_pool)(struct AllocatorContext *context, uint8_t *memory_pool, const size_t memory_pool_size);
    void (*freeze_permanent_arenas)(struct AllocatorContext *context);

} AllocatorPMVtable;

#pragma pack(pop)

inline void allocator_pm_set_memory_pool(AllocatorInterface *iface, uint8_t *memory_pool, const size_t memory_pool_size)
{
    ((AllocatorPMVtable *)iface->vtable)->set_memory_pool(iface->context, memory_pool, memory_pool_size);
}

inline void allocator_pm_freeze_permanent_arenas(AllocatorInterface *iface)
{
    ((AllocatorPMVtable *)iface->vtable)->freeze_permanent_arenas(iface->context);
}