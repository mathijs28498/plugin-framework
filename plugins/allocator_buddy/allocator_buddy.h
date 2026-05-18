#pragma once

#include <stdint.h>
#include <stddef.h>

#include <plugin_sdk/allocator/v1/allocator_types.h>

struct AllocatorContext;
typedef union AB_Handle
{
    struct
    {
        uint32_t generation;
        uint32_t index;
    };
    uint64_t raw;
} AB_Handle;

int32_t allocator_buddy_alloc(struct AllocatorContext *context, size_t size, AllocatorAllocationHandle *out_allocation_handle, void **out_arena);
int32_t allocator_buddy_free(struct AllocatorContext *context, AllocatorAllocationHandle allocation_handle);
void allocator_buddy_set_memory_pool(struct AllocatorContext *context, uint8_t *memory_pool, const size_t memory_pool_size);
void allocator_buddy_freeze_permanent_allocations(struct AllocatorContext *context);