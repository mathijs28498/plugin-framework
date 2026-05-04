#include "renderer_vulkan_register.h"

#include <assert.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include <plugin_sdk/renderer/v1/renderer_interface.h>
#include <plugin_sdk/arena_allocator/v1/arena_allocator_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_register, LOG_LEVEL_DEBUG)
#include <plugin_sdk/plugin_utils.h>

#include "renderer_vulkan.h"
#include "renderer_vulkan_start.h"
#include "renderer_vulkan_render.h"
#include "renderer_vulkan_cmd.h"
#include "renderer_vulkan_pipeline.h"

static const RendererVtable plugin_vtable = {
    .start = renderer_vulkan_start,
    .begin_frame = renderer_vulkan_render_begin_frame,
    .end_frame = renderer_vulkan_render_end_frame,
    .on_window_resize = renderer_vulkan_on_window_resize,

    .create_graphics_pipeline = renderer_vulkan_create_graphics_pipeline,
    .create_compute_pipeline = renderer_vulkan_create_compute_pipeline,

    .cmd_begin_render_pass = renderer_vulkan_cmd_begin_render_pass,
    .cmd_end_render_pass = renderer_vulkan_cmd_end_render_pass,
    .cmd_bind_pipeline = renderer_vulkan_cmd_bind_pipeline,
    .cmd_draw = renderer_vulkan_cmd_draw,
};

TODO("Figure out what these sizes should be");
TODO("Make these part of the configurations")
#define MAIN_DESTROY_QUEUE_CAPACITY 64
#define SWAPCHAIN_DESTROY_QUEUE_CAPACITY 64
#define FRAME_DESTROY_QUEUE_CAPACITY 32
#define SHADER_MODULES_CAPACITY 128
#define GRAPHICS_PIPELINES_CAPACITY 128
#define COMPUTE_PIPELINES_CAPACITY 128

static int32_t plugin_init(RendererContext *context)
{
    assert(context != NULL);
    int32_t ret;

    struct
    {
        INIT_ARRAY_MEMORY_FIELD(main_destroy_queue_mem, RV_CallRecord, MAIN_DESTROY_QUEUE_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(swapchain_destroy_queue_mem, RV_CallRecord, SWAPCHAIN_DESTROY_QUEUE_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(frame_destroy_queue_mem[ARRAY_SIZE(context->frames)], RV_CallRecord, FRAME_DESTROY_QUEUE_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(shader_modules_mem, VkShaderModule, SHADER_MODULES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(shader_module_generations_mem, uint32_t, SHADER_MODULES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(graphics_pipelines_mem, VkPipeline, GRAPHICS_PIPELINES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(graphics_pipelines_generations_mem, uint32_t, GRAPHICS_PIPELINES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(compute_pipelines_mem, VkPipeline, COMPUTE_PIPELINES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(compute_pipelines_generations_mem, uint32_t, COMPUTE_PIPELINES_CAPACITY);
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
    BIND_ARRAY_FILLED(VkShaderModule, arena->shader_modules_mem, context->shader_modules, SHADER_MODULES_CAPACITY);
    BIND_ARRAY_FILLED(uint32_t, arena->shader_module_generations_mem, context->shader_module_generations, SHADER_MODULES_CAPACITY);
    BIND_ARRAY_FILLED(VkPipeline, arena->graphics_pipelines_mem, context->graphics_pipelines, GRAPHICS_PIPELINES_CAPACITY);
    BIND_ARRAY_FILLED(uint32_t, arena->graphics_pipelines_generations_mem, context->graphics_pipeline_generations, GRAPHICS_PIPELINES_CAPACITY);
    BIND_ARRAY_FILLED(VkPipeline, arena->compute_pipelines_mem, context->compute_pipelines, COMPUTE_PIPELINES_CAPACITY);
    BIND_ARRAY_FILLED(uint32_t, arena->compute_pipelines_generations_mem, context->compute_pipeline_generations, COMPUTE_PIPELINES_CAPACITY);

    return 0;
}

static int32_t plugin_shutdown(RendererContext *context)
{
    renderer_vulkan_cleanup(context);

    return 0;
}

#include "plugin_register.c.inc"