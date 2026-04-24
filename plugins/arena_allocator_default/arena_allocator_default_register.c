#include "arena_allocator_default_register.h"

#include <plugin_sdk/arena_allocator/v1/arena_allocator_interface.h>
#include <plugin_sdk/arena_allocator/v1/arena_allocator_pm_interface.h>
#include <plugin_sdk/plugin_utils.h>

#include "arena_allocator_default.h"

static const ArenaAllocatorPMVtable plugin_vtable = {
    .public_vtable = {
        .get_arena = arena_allocator_default_get_arena,
    },
    .set_memory_pool = arena_allocator_default_set_memory_pool,
    .freeze_permanent_arenas = arena_allocator_default_freeze_permanent_arenas,
};

#include "plugin_register.c.inc"