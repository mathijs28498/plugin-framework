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
#include "renderer_vulkan_descriptor_set.h"

void renderer_vulkan_dummy_get_extent(RendererContext *context, uint32_t extent[2])
{
    assert(context != NULL);
    assert(extent != NULL);

    extent[0] = context->draw_extent.width;
    extent[1] = context->draw_extent.height;
}
RendererResourceSetLayoutHandle renderer_vulkan_dummy_get_draw_image_ds_handle(RendererContext *context)
{
    return context->draw_image_descriptor_set_layout_handle;
}

static const RendererVtable plugin_vtable = {
    .dummy_get_extent = renderer_vulkan_dummy_get_extent,

    .start = renderer_vulkan_start,
    .begin_frame = renderer_vulkan_render_begin_frame,
    .end_frame = renderer_vulkan_render_end_frame,
    .on_window_resize = renderer_vulkan_on_window_resize,

    .create_shader = renderer_vulkan_create_shader,
    .destroy_shader = renderer_vulkan_destroy_shader,

    .create_resource_set_layout = renderer_vulkan_create_resource_set_layout,
    .allocate_transient_resource_set = renderer_vulkan_allocate_transient_resource_set,
    .update_transient_resource_set = renderer_vulkan_update_transient_resource_set,

    .create_pipeline_layout = renderer_vulkan_create_pipeline_layout,

    .create_graphics_pipeline = renderer_vulkan_create_graphics_pipeline,
    .create_compute_pipeline = renderer_vulkan_create_compute_pipeline,

    .cmd_end_rendering = renderer_vulkan_cmd_end_rendering,
    .cmd_bind_graphics_pipeline = renderer_vulkan_cmd_bind_graphics_pipeline,
    .cmd_bind_compute_pipeline = renderer_vulkan_cmd_bind_compute_pipeline,
    .cmd_draw = renderer_vulkan_cmd_draw,
    .cmd_bind_resource_sets = renderer_vulkan_cmd_bind_resource_sets,
    .cmd_push_constants = renderer_vulkan_cmd_push_constants,
    .cmd_dispatch = renderer_vulkan_cmd_dispatch,
};

TODO("Figure out what these sizes should be");
TODO("Make these part of the configurations")
#define MAIN_DESTROY_QUEUE_CAPACITY 64
#define SWAPCHAIN_DESTROY_QUEUE_CAPACITY 64
#define FRAME_DESTROY_QUEUE_CAPACITY 32
#define SHADER_MODULES_CAPACITY 128
#define DESCRIPTOR_SET_LAYOUTS_CAPACITY 128
#define TRANSIENT_DESCRIPTOR_SETS_CAPACITY 128
#define PIPELINES_CAPACITY 128
#define PIPELINE_LAYOUTS_CAPACITY 128
#define ALLOCATED_IMAGES_CAPACITY 128
#define BUMP_ARENA_CAPACITY (1024 * 128)

static int32_t plugin_init(RendererContext *context)
{
    assert(context != NULL);
    int32_t ret;

    struct
    {
        INIT_ARRAY_MEMORY_FIELD(main_destroy_queue_mem, RV_CallRecord, MAIN_DESTROY_QUEUE_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(swapchain_destroy_queue_mem, RV_CallRecord, SWAPCHAIN_DESTROY_QUEUE_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(frame_destroy_queue_mem[ARRAY_SIZE(context->frames)], RV_CallRecord, FRAME_DESTROY_QUEUE_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(transient_descriptor_sets_mem[ARRAY_SIZE(context->frames)], VkDescriptorSet, TRANSIENT_DESCRIPTOR_SETS_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(shader_module_occupied_mem, bool, SHADER_MODULES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(shader_module_generations_mem, uint32_t, SHADER_MODULES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(shader_modules_mem, VkShaderModule, SHADER_MODULES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(descriptor_set_layout_occupied_mem, bool, DESCRIPTOR_SET_LAYOUTS_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(descriptor_set_layout_generations_mem, uint32_t, DESCRIPTOR_SET_LAYOUTS_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(descriptor_set_layouts_mem, VkDescriptorSetLayout, DESCRIPTOR_SET_LAYOUTS_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(pipeline_layout_occupied_mem, bool, PIPELINE_LAYOUTS_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(pipeline_layout_generations_mem, uint32_t, PIPELINE_LAYOUTS_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(pipeline_layouts_mem, VkPipelineLayout, PIPELINE_LAYOUTS_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(pipeline_occupied_mem, bool, PIPELINES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(pipeline_generations_mem, uint32_t, PIPELINES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(pipelines_mem, VkPipeline, PIPELINES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(allocated_image_occupied_mem, bool, ALLOCATED_IMAGES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(allocated_image_generations_mem, uint32_t, ALLOCATED_IMAGES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(allocated_images_mem, RV_AllocatedImage, ALLOCATED_IMAGES_CAPACITY);
        INIT_ARRAY_MEMORY_FIELD(bump_arena_mem, uint8_t, BUMP_ARENA_CAPACITY);
    } *arena;

    size_t alloc_size = sizeof(*arena);
    RETURN_IF_ERROR(context->deps.logger, ret, arena_allocator_get_arena(context->deps.arena_allocator, alloc_size, &arena),
                    "Unable to allocate arena allocator: %d", -1);

    BIND_ARRAY(RV_CallRecord, arena->main_destroy_queue_mem, context->main_destroy_queue, MAIN_DESTROY_QUEUE_CAPACITY);
    BIND_ARRAY(RV_CallRecord, arena->swapchain_destroy_queue_mem, context->swapchain_destroy_queue, SWAPCHAIN_DESTROY_QUEUE_CAPACITY);
    for (size_t i = 0; i < ARRAY_SIZE(context->frames); i++)
    {
        BIND_ARRAY(RV_CallRecord, arena->frame_destroy_queue_mem[i], context->frames[i].destroy_queue, FRAME_DESTROY_QUEUE_CAPACITY);
        BIND_ARRAY(VkDescriptorSet, arena->transient_descriptor_sets_mem[i], context->frames[i].transient_descriptor_sets, TRANSIENT_DESCRIPTOR_SETS_CAPACITY);
    }

    BIND_ARRAY_FILLED(bool, arena->shader_module_occupied_mem, context->shader_module_occupied_a, SHADER_MODULES_CAPACITY);
    BIND_ARRAY_FILLED(uint32_t, arena->shader_module_generations_mem, context->shader_module_generations_a, SHADER_MODULES_CAPACITY);
    BIND_ARRAY_FILLED(VkShaderModule, arena->shader_modules_mem, context->shader_modules_a, SHADER_MODULES_CAPACITY);

    BIND_ARRAY_FILLED(bool, arena->descriptor_set_layout_occupied_mem, context->descriptor_set_layout_occupied_a, DESCRIPTOR_SET_LAYOUTS_CAPACITY);
    BIND_ARRAY_FILLED(uint32_t, arena->descriptor_set_layout_generations_mem, context->descriptor_set_layout_generations_a, DESCRIPTOR_SET_LAYOUTS_CAPACITY);
    BIND_ARRAY_FILLED(VkDescriptorSetLayout, arena->descriptor_set_layouts_mem, context->descriptor_set_layouts_a, DESCRIPTOR_SET_LAYOUTS_CAPACITY);

    BIND_ARRAY_FILLED(bool, arena->pipeline_layout_occupied_mem, context->pipeline_layout_occupied_a, PIPELINE_LAYOUTS_CAPACITY);
    BIND_ARRAY_FILLED(uint32_t, arena->pipeline_layout_generations_mem, context->pipeline_layout_generations_a, PIPELINE_LAYOUTS_CAPACITY);
    BIND_ARRAY_FILLED(VkPipelineLayout, arena->pipeline_layouts_mem, context->pipeline_layouts_a, PIPELINE_LAYOUTS_CAPACITY);

    BIND_ARRAY_FILLED(bool, arena->pipeline_occupied_mem, context->pipeline_occupied_a, PIPELINES_CAPACITY);
    BIND_ARRAY_FILLED(uint32_t, arena->pipeline_generations_mem, context->pipeline_generations_a, PIPELINES_CAPACITY);
    BIND_ARRAY_FILLED(VkPipeline, arena->pipelines_mem, context->pipelines_a, PIPELINES_CAPACITY);

    BIND_ARRAY_FILLED(bool, arena->allocated_image_occupied_mem, context->allocated_image_occupied_a, PIPELINES_CAPACITY);
    BIND_ARRAY_FILLED(uint32_t, arena->pipeline_generations_mem, context->allocated_image_generations_a, PIPELINES_CAPACITY);
    BIND_ARRAY_FILLED(RV_AllocatedImage, arena->allocated_images_mem, context->allocated_images_a, PIPELINES_CAPACITY);

    BIND_ARRAY(uint8_t, arena->bump_arena_mem, context->bump_arena_a, BUMP_ARENA_CAPACITY);

    return 0;
}

static int32_t plugin_shutdown(RendererContext *context)
{
    renderer_vulkan_cleanup(context);

    return 0;
}

#include "plugin_register.c.inc"