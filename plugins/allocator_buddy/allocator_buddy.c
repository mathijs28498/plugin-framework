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

TODO("Add unit tests");

#define ALLOCATION_PAGE_SIZE 128ULL
#define ALLOCATION_HANDLE_NO_HANDLE 0xFFFFFFFFFFFFFFFF
#define OCCUPIED_NODE_VALUE 0xFF
_Static_assert(OCCUPIED_NODE_VALUE == 0xFF, "OCCUPIED_NODE_VALUE needs to be exactly 0xFF, the overflow value is used for the tree");

#define ALIGN_WITH_ALLOCATION_PAGE(value) \
    (((value) + ((uintptr_t)(ALLOCATION_PAGE_SIZE) - 1)) & ~((uintptr_t)(ALLOCATION_PAGE_SIZE) - 1))

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

uint8_t get_max_node_order(uint8_t max_depth, uint32_t idx)
{
    return max_depth - find_msb((uint64_t)idx + 1ULL) - 1;
}

int32_t allocate_permanent(AllocatorContext *context, size_t size, void **out_allocation)
{
    size_t occupied_size = ALIGN_WITH_ALLOCATION_PAGE(size);

    if (context->memory_pool_size < occupied_size)
    {
        LOG_ERR_TRACE(context->deps.logger, "Not enough space for allocating permanent allocation: requested '%zu', size '%zu'", occupied_size, context->memory_pool_size);
        return -1;
    }

    memset(context->memory_pool, 0, occupied_size);

    *out_allocation = context->memory_pool;
    context->memory_pool += occupied_size;
    context->memory_pool_size -= occupied_size;

    return 0;
}

void print_node_fragmentation(AllocatorContext *context, uint32_t node_idx, uint8_t level, char *buffer, size_t *offset, size_t buf_size)
{
    uint8_t value = context->free_list_tree[node_idx];
    uint8_t total_levels = context->free_list_depth;
    uint8_t current_order = total_levels - 1 - level;

    if (value == current_order)
    {
        uint64_t free_bytes = (1ULL << current_order) * ALLOCATION_PAGE_SIZE;

        int width = (current_order * 6) + 4;
        *offset += snprintf(buffer + *offset, buf_size - *offset, "%*lluB | ", width, free_bytes);
        return;
    }

    if (value == OCCUPIED_NODE_VALUE)
    {
        int width = (current_order * 6) + 4;

        *offset += snprintf(buffer + *offset, buf_size - *offset, "%*sXXXX | ", width, "");
        return;
    }

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

    print_node_fragmentation(context, 0, 0, buffer, &offset, sizeof(buffer));

    LOG_INF(context->deps.logger, "Memory Map Fragmentation Display:");
    LOG_INF(context->deps.logger, "%s", buffer);
}

void *buddy_index_to_address(AllocatorContext *context, uint32_t idx)
{
    assert(context != NULL);

    uint8_t level = find_msb((uint64_t)idx + 1);

    uint32_t pos_in_level = idx - ((1u << level) - 1u);

    uint8_t order = (context->free_list_depth - 1) - level;
    size_t block_size = (size_t)1 << (order + context->page_size_msb);

    size_t offset = (size_t)pos_in_level * block_size;

    return (void *)(context->memory_pool + offset);
}

int32_t allocate_buddy(AllocatorContext *context, size_t size, AllocatorAllocationHandle *out_allocation_handle, void **out_allocation)
{
    assert(context != NULL);
    assert(size > 0);
    assert(out_allocation_handle != NULL);
    assert(out_allocation != NULL);

    LoggerInterface *logger = context->deps.logger;

    // Remove 1 from the size to solve for sizes that are a power of 2 exactly
    // Adding 1 to the msb to give the smallest power of 2 that the size fits in entirely
    uint8_t aligned_size_msb = find_msb((uint64_t)size - 1) + 1;

    uint8_t node_order = (aligned_size_msb < context->page_size_msb) ? 0 : aligned_size_msb - context->page_size_msb;
    uint64_t allocated_size = 1ULL << (node_order + context->page_size_msb);

    uint8_t node_depth = context->free_list_depth - node_order;

    // Finding the buddy to allocate
    // We loop over the node depth - 1 as the found node in the final iteration is the node to allocate
    uint32_t idx = 0;
    for (uint8_t i = 0; i < node_depth - 1; i++)
    {
        uint8_t current_value = context->free_list_tree[idx];
        RETURN_IF_TRUE(logger, current_value < node_order,
                       -1, "No space available for current required size: %d, max available: %d", allocated_size, 1ULL << (current_value + context->page_size_msb));
        uint32_t left_idx = idx * 2 + 1;
        uint8_t left_value = context->free_list_tree[left_idx];

        if (left_value != OCCUPIED_NODE_VALUE && left_value >= node_order)
        {
            idx = left_idx;
            continue;
        }

        uint32_t right_idx = idx * 2 + 2;
        uint8_t right_value = context->free_list_tree[right_idx];
        if (right_value != OCCUPIED_NODE_VALUE && right_value >= node_order)
        {
            idx = right_idx;
            continue;
        }

        UNREACHABLE();
        LOG_ERR_TRACE(logger, "Somehow no suitable child in buddy allocator found, this should not happen");
        return -1;
    }

    context->free_list_tree[idx] = OCCUPIED_NODE_VALUE;
    uint32_t parent_idx = idx;
    TODO("Replace this loop with SAFE_WHILE");
    do
    {
        parent_idx = (parent_idx - 1) / 2;
        // Adding 1 to the value to over flow OCCUPIED members, as OCCUPIED is 0xFF
        uint8_t left_value = context->free_list_tree[parent_idx * 2 + 1] + 1;
        uint8_t right_value = context->free_list_tree[parent_idx * 2 + 2] + 1;
        context->free_list_tree[parent_idx] = MAX(left_value, right_value) - 1;
    } while (parent_idx != 0);

    debug_log_buddy_fragmentation(context);

    *out_allocation = buddy_index_to_address(context, idx);
    TODO("Make generation work")
    AB_Handle handle = {
        .generation = 0,
        .index = idx,
    };
    *out_allocation_handle = handle.raw;
    return 0;
}

TODO("Get alloc, keep track of what allocations are taken out");
TODO("Allow user to give an id that needs to be registered first");
int32_t allocator_buddy_alloc(AllocatorContext *context, size_t size, AllocatorAllocationHandle *allocation_handle, void **out_allocation)
{
    assert(context != NULL);
    assert(size > 0);
    assert(allocation_handle != NULL);
    assert(out_allocation != NULL);

    *allocation_handle = ALLOCATION_HANDLE_NO_HANDLE;
    *out_allocation = NULL;

    if (!context->permanent_allocations_frozen)
    {
        return allocate_permanent(context, size, out_allocation);
    }

    return allocate_buddy(context, size, allocation_handle, out_allocation);
}

int32_t free_buddy(AllocatorContext *context, AllocatorAllocationHandle allocation_handle)
{
    assert(context != NULL);

    LoggerInterface *logger = context->deps.logger;

    AB_Handle ab_allocation_handle = {.raw = allocation_handle};
    TODO("Do oob check on handle index")

    uint8_t node_value = context->free_list_tree[ab_allocation_handle.index];
    RETURN_IF_TRUE(logger, node_value != OCCUPIED_NODE_VALUE,
                   -1, "Allocation has not been allocated, idx: %d, value: %d", ab_allocation_handle.index, node_value);

    TODO("Fix parents")
    context->free_list_tree[ab_allocation_handle.index] = get_max_node_order(context->free_list_depth, ab_allocation_handle.index);

    uint32_t parent_idx = ab_allocation_handle.index;
    TODO("Replace this loop with SAFE_WHILE");
    do
    {
        parent_idx = (parent_idx - 1) / 2;
        uint8_t parent_max_order = get_max_node_order(context->free_list_depth, parent_idx);

        uint8_t left_value = context->free_list_tree[parent_idx * 2 + 1];
        uint8_t right_value = context->free_list_tree[parent_idx * 2 + 2];

        uint8_t new_value;
        if (left_value == parent_max_order - 1 && right_value == parent_max_order - 1)
        {
            new_value = parent_max_order;
        }
        else
        {
            // Adding 1 to the value to over flow OCCUPIED members, as OCCUPIED is 0xFF
            left_value++;
            right_value++;
            new_value = MAX(left_value, right_value) - 1;
        }
        context->free_list_tree[parent_idx] = new_value;
    } while (parent_idx != 0);

    debug_log_buddy_fragmentation(context);

    return 0;
}

int32_t allocator_buddy_free(AllocatorContext *context, AllocatorAllocationHandle allocation_handle)
{
    assert(context != NULL);

    if (allocation_handle == ALLOCATION_HANDLE_NO_HANDLE)
    {
        LOG_DBG(context->deps.logger, "Attempted to free with no handle");
        return 0;
    }

    return free_buddy(context, allocation_handle);
}

void allocator_buddy_set_memory_pool(AllocatorContext *context, uint8_t *memory_pool, const size_t memory_pool_size)
{
    assert(context != NULL);
    assert(memory_pool != NULL);

    uintptr_t current_addr = (uintptr_t)memory_pool;
    uintptr_t aligned_addr = ALIGN_WITH_ALLOCATION_PAGE(current_addr);
    size_t padding = aligned_addr - current_addr;

    context->memory_pool = (uint8_t *)aligned_addr;
    context->memory_pool_size = memory_pool_size - padding;
}

TODO("Make sure you dont waste up to almost half the space")
TODO("Divide the rest differently")
void allocator_buddy_freeze_permanent_allocations(AllocatorContext *context)
{
    assert(context != NULL);
    assert(context->memory_pool_size >= 2 * ALLOCATION_PAGE_SIZE);

    uint8_t size_msb = find_msb(context->memory_pool_size);
    uint64_t new_size = 1ULL << size_msb;
    uint64_t padding = context->memory_pool_size - new_size;

    uint8_t page_size_msb = find_msb(ALLOCATION_PAGE_SIZE);
    uint8_t msb_diff = size_msb - page_size_msb;
    uint64_t free_list_size = (1ULL << msb_diff) * 2 - 1;
    uint64_t aligned_free_list_size = ALIGN_WITH_ALLOCATION_PAGE(free_list_size);

    if (padding < aligned_free_list_size)
    {
        new_size >>= 1;

        msb_diff -= 1;
        free_list_size = (1ULL << msb_diff) * 2 - 1;
        aligned_free_list_size = ALIGN_WITH_ALLOCATION_PAGE(free_list_size);
    }

    LOG_DBG(context->deps.logger, "New allocator size: %d, depth: %d, aligned_free_list_size: %d", new_size, msb_diff, aligned_free_list_size);
    context->page_size_msb = page_size_msb;

    context->free_list_tree = context->memory_pool;
    context->free_list_depth = msb_diff + 1;

    context->memory_pool = (uint8_t *)((uintptr_t)context->memory_pool + aligned_free_list_size);
    context->memory_pool_size = new_size;

    context->permanent_allocations_frozen = true;

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