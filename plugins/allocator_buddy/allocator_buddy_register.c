#include "allocator_buddy_register.h"

#include <plugin_sdk/allocator/v1/allocator_interface.h>
#include <plugin_sdk/allocator/v1/allocator_pm_interface.h>
#include <plugin_sdk/plugin_utils.h>

#include "allocator_buddy.h"

static const AllocatorPMVtable plugin_vtable = {
    .public_vtable = {
        .alloc = allocator_buddy_alloc,
    },
    .set_memory_pool = allocator_buddy_set_memory_pool,
    .freeze_permanent_arenas = allocator_buddy_freeze_permanent_arenas,
};

#include "plugin_register.c.inc"