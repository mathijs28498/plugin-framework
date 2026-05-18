#include "allocator_buddy.h"

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(allocator_buddy, LOG_LEVEL_DEBUG)
#include <plugin_sdk/allocator/v1/allocator_interface.h>
#include <plugin_sdk/allocator/v1/allocator_pm_interface.h>
#include <plugin_sdk/allocator/v1/allocator_types.h>
#include <plugin_sdk/plugin_utils.h>

#include "allocator_buddy_register.h"

#define ARENA_PAGE_SIZE 128ULL
#define ALLOCATION_HANDLE_NO_HANDLE 0xFFFFFFFFFFFFFFFF

#define ALIGN_WITH_ARENA_PAGE(value) \
    (((value) + ((uintptr_t)(ARENA_PAGE_SIZE) - 1)) & ~((uintptr_t)(ARENA_PAGE_SIZE) - 1))

int32_t allocate_arena_permanent(AllocatorContext *context, size_t size, void **out_arena)
{
    size_t occupied_size = ALIGN_WITH_ARENA_PAGE(size);

    if (context->memory_pool_size < occupied_size)
    {
        LOG_ERR_TRACE(context->deps.logger, "Not enough space for allocating permanent arena: requested '%zu', size '%zu'", occupied_size, context->memory_pool_size);
        return -1;
    }

    memset(context->memory_pool, 0, occupied_size);

    *out_arena = context->memory_pool;
    context->memory_pool += occupied_size;
    context->memory_pool_size -= occupied_size;

    return 0;
}

TODO("Optimize")
uint8_t find_msb(uint64_t num)
{
    static const uint8_t bit_len = sizeof(num) * CHAR_BIT - 1;
    for (uint8_t i = 0; i < bit_len; i++)
    {
        uint8_t bit_shft = bit_len - i;

        if ((num & (1ULL << bit_shft)) != 0)
        {
            return bit_shft;
        }
    }
    return 0;
}

void log_buddy_arena(AllocatorContext *context)
{
    assert(context != NULL);
}

void log_value(AllocatorContext *context, uint8_t idx)
{
    LOG_DBG(context->deps.logger, "idx: %d, value: %d", idx, context->free_list_tree[idx]);
}

void print_node_fragmentation(AllocatorContext *context, uint32_t node_idx, uint8_t level, char *buffer, size_t *offset, size_t buf_size)
{
    uint8_t value = context->free_list_tree[node_idx];
    uint8_t total_levels = context->free_list_depth;
    uint8_t current_order = total_levels - 1 - level;

    // Rule 1: If the node is completely free, its value equals its theoretical maximum capacity order.
    // We can print it as one big unallocated chunk and skip looking at its children.
    if (value == current_order)
    {
        uint64_t free_bytes = (1ULL << current_order) * ARENA_PAGE_SIZE;

        // Dynamic padding: Larger blocks get a wider visual slot
        int width = (current_order * 6) + 4;
        *offset += snprintf(buffer + *offset, buf_size - *offset, "%*lluB | ", width, free_bytes);
        return;
    }

    // Rule 2: If the node is explicitly allocated (255), print its size block
    if (value == 255)
    {
        int width = (current_order * 6) + 4;

        // Print 'X' padding inside the bracket to show it is filled
        *offset += snprintf(buffer + *offset, buf_size - *offset, "%*sXXXX | ", width, "");
        return;
    }

    // Rule 3: The node has a value smaller than its maximum order, meaning it is split.
    // Recurse into the children to show exactly where the fragmentation lives.
    uint32_t left_child = node_idx * 2 + 1;
    uint32_t right_child = node_idx * 2 + 2;

    print_node_fragmentation(context, left_child, level + 1, buffer, offset, buf_size);
    print_node_fragmentation(context, right_child, level + 1, buffer, offset, buf_size);
}

void debug_log_buddy_fragmentation(AllocatorContext *context)
{
    assert(context != NULL);
    assert(context->free_list_tree != NULL);

    char buffer[1024] = {0};
    size_t offset = 0;

    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "| ");

    // Kick off the recursion from the root node (index 0, level 0)
    print_node_fragmentation(context, 0, 0, buffer, &offset, sizeof(buffer));

    LOG_INF(context->deps.logger, "Memory Map Fragmentation Display:");
    LOG_INF(context->deps.logger, "%s", buffer);
}

int32_t allocate_arena_buddy(AllocatorContext *context, size_t size, AllocatorAllocationHandle *out_allocation_handle, void **out_arena)
{
    assert(context != NULL);
    assert(size > 0);
    assert(out_allocation_handle != NULL);
    assert(out_arena != NULL);

    LoggerInterface *logger = context->deps.logger;

    // Remove 1 from the size to solve for sizes that are a power of 2 exactly
    // Adding 1 to the msb to give the smallest power of 2 that the size fits in entirely
    uint8_t aligned_size_msb = find_msb((uint64_t)size - 1) + 1;
    uint64_t aligned_size = 1ULL << aligned_size_msb;
    uint8_t node_order = (aligned_size_msb < context->page_size_msb)
                             ? 0
                             : aligned_size_msb - context->page_size_msb;
    // uint8_t node_depth = context->free_list_depth - node_order;

    TODO("Remove this")
    uint64_t allocated_size = 1ULL << (node_order + context->page_size_msb);

    // RETURN_IF_TRUE(logger, allocated_size > context->memory_pool_size,
    //                -1, "Failed to allocate arena, requested size is bigger than size available, requested: %d, allocated: %d, actual: %d", size, allocated_size, context->memory_pool_size);

    uint8_t node_depth = context->free_list_depth - node_order;

    LOG_WRN(logger, "Starting allocation\nrequested size: %d\naligned size: %d\nallocated size: %d\nnode depth: %d\nnode order: %d\nmemory pool size: %d",
            size, aligned_size, allocated_size, node_depth, node_order, context->memory_pool_size);
    log_buddy_arena(context);

    uint8_t idx = 0;

    // Finding the buddy to allocate
    // We loop over the node depth - 1 as the found node in the final iteration is the node to allocate
    for (uint8_t i = 0; i < node_depth - 1; i++)
    {
        uint8_t current_value = context->free_list_tree[idx];
        RETURN_IF_TRUE(logger, current_value < node_order,
                       -1, "No space available for current required size: %d, max available: %d", allocated_size, 1ULL << (current_value + context->page_size_msb));
        uint8_t left_idx = idx * 2 + 1;
        uint8_t left_value = context->free_list_tree[left_idx];

        if (left_value != 0xFF && left_value >= node_order)
        {
            LOG_DBG(logger, "Choosing left");
            idx = left_idx;
            continue;
        }

        uint8_t right_idx = idx * 2 + 2;
        uint8_t right_value = context->free_list_tree[right_idx];
        if (right_value != 0xFF && right_value >= node_order)
        {
            LOG_DBG(logger, "Choosing right");
            idx = right_idx;
            continue;
        }

        LOG_ERR_TRACE(logger, "Somehow no suitable child in buddy allocator found, this should not happen");
        return -1;
    }

    LOG_INF(logger, "idx: %d", idx);
    context->free_list_tree[idx] = 0xFF;
    uint8_t parent_idx = idx;
    do
    {
        parent_idx = (parent_idx - 1) / 2;
        uint8_t left_value = context->free_list_tree[parent_idx * 2 + 1] + 1;
        uint8_t right_value = context->free_list_tree[parent_idx * 2 + 2] + 1;
        context->free_list_tree[parent_idx] = MAX(left_value, right_value) - 1;
    } while (parent_idx != 0);

    debug_log_buddy_fragmentation(context);

    TODO("Make generation work")
    AAD_Handle handle = {
        .generation = 0,
        .index = (uint32_t)idx,
    };
    *out_allocation_handle = handle.raw;
    return 0;
}

TODO("Get arena, keep track of what arenas are taken out");
TODO("Allow user to give an id that needs to be registered first");
int32_t allocator_buddy_alloc(AllocatorContext *context, size_t size, AllocatorAllocationHandle *allocation_handle, void **out_arena)
{
    assert(context != NULL);
    assert(size > 0);
    assert(allocation_handle != NULL);
    assert(out_arena != NULL);

    *allocation_handle = ALLOCATION_HANDLE_NO_HANDLE;
    *out_arena = NULL;

    if (!context->permanent_arenas_frozen)
    {
        return allocate_arena_permanent(context, size, out_arena);
    }

    return allocate_arena_buddy(context, size, allocation_handle, out_arena);
}

void allocator_buddy_set_memory_pool(AllocatorContext *context, uint8_t *memory_pool, const size_t memory_pool_size)
{
    assert(context != NULL);
    assert(memory_pool != NULL);

    uintptr_t current_addr = (uintptr_t)memory_pool;
    uintptr_t aligned_addr = ALIGN_WITH_ARENA_PAGE(current_addr);
    size_t padding = aligned_addr - current_addr;

    context->memory_pool = (uint8_t *)aligned_addr;
    context->memory_pool_size = memory_pool_size - padding;
}

TODO("Make sure you dont waste up to almost half the space")
TODO("Divide the rest differently")
void allocator_buddy_freeze_permanent_arenas(AllocatorContext *context)
{
    assert(context != NULL);
    assert(context->memory_pool_size >= 2 * ARENA_PAGE_SIZE);

    uint8_t size_msb = find_msb(context->memory_pool_size);
    uint64_t new_size = 1ULL << size_msb;
    uint64_t padding = context->memory_pool_size - new_size;

    uint8_t page_size_msb = find_msb(ARENA_PAGE_SIZE);
    uint8_t msb_diff = size_msb - page_size_msb;
    LOG_DBG(context->deps.logger, "size_msb: %d, page_size_msb: %d, msb_diff: %d", size_msb, page_size_msb, msb_diff);
    uint64_t free_list_size = (1ULL << msb_diff) * 2 - 1;
    uint64_t aligned_free_list_size = ALIGN_WITH_ARENA_PAGE(free_list_size);

    if (padding < aligned_free_list_size)
    {
        new_size >>= 1;

        msb_diff -= 1;
        free_list_size = (1ULL << msb_diff) * 2 - 1;
        aligned_free_list_size = ALIGN_WITH_ARENA_PAGE(free_list_size);
    }

    LOG_DBG(context->deps.logger, "New allocator size: %d, depth: %d, aligned_free_list_size: %d", new_size, msb_diff, aligned_free_list_size);

    context->page_size_msb = page_size_msb;

    TODO("Make this uint8_t again");
    context->free_list_tree = context->memory_pool;
    context->free_list_depth = msb_diff + 1;

    context->memory_pool = (uint8_t *)((uintptr_t)context->memory_pool + aligned_free_list_size);
    context->memory_pool_size = new_size;

    context->permanent_arenas_frozen = true;

    uint32_t idx = 0;
    for (uint8_t i = 0; i < context->free_list_depth; i++)
    {
        uint8_t max_free_size = msb_diff - i;

        for (uint32_t j = 0; j < (1U << i); j++)
        {
            context->free_list_tree[idx] = max_free_size;
            idx++;
        }
    }
}