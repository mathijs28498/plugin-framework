#include "renderer_vulkan_register.h"

#include <assert.h>
#include <stdint.h>

#include <plugin_sdk/renderer/v1/renderer_interface.h>
#include <plugin_sdk/arena_allocator/v1/arena_allocator_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_register, LOG_LEVEL_DEBUG)
#include <plugin_sdk/plugin_utils.h>

#include "renderer_vulkan.h"
#include "renderer_vulkan_start.h"
#include "renderer_vulkan_render.h"

static const RendererVtable plugin_vtable = {
    .start = renderer_vulkan_start,
    .begin_frame = renderer_vulkan_render_begin_frame,
    .end_frame = renderer_vulkan_render_end_frame,
    .render = renderer_vulkan_render,
    .on_window_resize = renderer_vulkan_on_window_resize,
};

// #define ALLOC_ARRAY(type, arr_ptr, var_name, cap)

TODO("Figure out what these sizes should be");
#define MAIN_DESTROY_QUEUE_CAPACITY 64
#define SWAPCHAIN_DESTROY_QUEUE_CAPACITY 64
#define FRAME_DESTROY_QUEUE_CAPACITY 32

static int32_t plugin_init(RendererContext *context)
{
    assert(context != NULL);
    int32_t ret;

    struct
    {
        _Alignas(ArrayHeader_) uint8_t main_destroy_queue_mem[ARRAY_MEMORY_SIZE(RV_CallRecord, MAIN_DESTROY_QUEUE_CAPACITY)];
        _Alignas(ArrayHeader_) uint8_t swapchain_destroy_queue_mem[ARRAY_MEMORY_SIZE(RV_CallRecord, SWAPCHAIN_DESTROY_QUEUE_CAPACITY)];
        _Alignas(ArrayHeader_) uint8_t frame_destroy_queue_mem[ARRAY_SIZE(context->frames)][ARRAY_MEMORY_SIZE(RV_CallRecord, FRAME_DESTROY_QUEUE_CAPACITY)];
    } *arena;

    size_t alloc_size = sizeof(*arena);
    RETURN_IF_ERROR(context->deps.logger, ret, arena_allocator_get_arena(context->deps.arena_allocator, alloc_size, &arena),
                    "Unable to allocate arena allocator: %d", -1);

    BIND_ARRAY(RV_CallRecord, arena->main_destroy_queue_mem, context->main_destroy_queue, MAIN_DESTROY_QUEUE_CAPACITY);
    BIND_ARRAY(RV_CallRecord, arena->swapchain_destroy_queue_mem, context->swapchain_destroy_queue, SWAPCHAIN_DESTROY_QUEUE_CAPACITY);
    for (size_t i = 0; i < ARRAY_SIZE(context->frames); i++)
    {
        BIND_ARRAY(RV_CallRecord, arena->frame_destroy_queue_mem[i], context->frames[i].destroy_queue, FRAME_DESTROY_QUEUE_CAPACITY);
    }

    return 0;
}

static int32_t plugin_shutdown(RendererContext *context)
{
    renderer_vulkan_cleanup(context);

    return 0;
}

#include "plugin_register.c.inc"