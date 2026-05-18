#pragma once

#include <stdint.h>
#include <stddef.h>

#include "allocator_types.h"
#include "../../plugin_macros.h"

typedef struct AllocatorContext AllocatorContext;

#pragma pack(push, 8)

typedef struct AllocatorVtable
{
    int32_t (*alloc)(AllocatorContext *context, size_t size, AllocatorAllocationHandle *out_allocation_handle, void **out_arena);
    int32_t (*free)(AllocatorContext *context, AllocatorAllocationHandle allocation_handle);
    int32_t (*register_pools)(AllocatorContext *context, uint32_t *id);
} AllocatorVtable;

typedef struct AllocatorInterface
{
    struct AllocatorContext *context;
    const AllocatorVtable *vtable;
} AllocatorInterface;

#pragma pack(pop)

static inline int32_t allocator_alloc(AllocatorInterface *iface, size_t size, AllocatorAllocationHandle *out_allocation_handle, void **out_arena)
{
    return VTABLE_METHOD_CALL(iface, alloc, size, out_allocation_handle, out_arena);
}

static inline int32_t allocator_free(AllocatorInterface *iface, AllocatorAllocationHandle allocation_handle)
{
    return VTABLE_METHOD_CALL(iface, free, allocation_handle);
}

static inline int32_t allocator_register_pools(AllocatorInterface *iface, uint32_t *id)
{
    return VTABLE_METHOD_CALL(iface, register_pools, id);
}