#include "draw_default.h"

#include <stdint.h>
#include <assert.h>
#include <math.h>

#include <cglm/types.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(draw_default, LOG_LEVEL_DEBUG);
#include <plugin_sdk/renderer/v1/renderer_interface.h>
#include <plugin_sdk/plugin_utils.h>

#include <plugin_sdk/plugin_utils.h>

#include "shader_background_compute.h"

#include "draw_default_register.h"

typedef struct BackgroundPushConstants
{
    vec4 top_left;
    vec4 top_right;
    vec4 bottom_left;
    vec4 bottom_right;
} BackgroundPushConstants;

int32_t create_resource_set_layouts(DrawContext *context)
{
    assert(context != NULL);
    int32_t ret;

    RendererResourceSetLayoutBinding resource_set_layout_bindings[] = {
        {
            .binding = 0,
            .resource_len = 1,
            .resource_type = RENDERER_RESOURCE_TYPE_STORAGE_IMAGE,
            .stage_flags = RENDERER_SHADER_STAGE_COMPUTE_BIT,
        }};

    RendererResourceSetLayoutCreateInfo resource_set_layout_create_info = {
        .bindings_len = ARRAY_SIZE(resource_set_layout_bindings),
        .bindings = resource_set_layout_bindings,
    };

    RETURN_IF_ERROR(context->deps.logger, ret, renderer_create_resource_set_layout(context->deps.renderer, &resource_set_layout_create_info, &context->draw_image_resource_set_layout_handle),
                    "Failed to create resource set: %d", ret);

    // {(VkDescriptorSetLayoutBinding){
    //     .binding = 0,
    //     .descriptorCount = 1,
    //     .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    //     .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    // }});

    return 0;
}

int32_t draw_default_start(DrawContext *context)
{
    assert(context != NULL);
    int32_t ret;

    RETURN_IF_ERROR(context->deps.logger, ret, renderer_start(context->deps.renderer),
                    "Error initializing renderer: %d", ret);

    RETURN_IF_ERROR(context->deps.logger, ret, create_resource_set_layouts(context),
                    "Failed to create resource set layouts: %d", ret);

    RendererShaderHandle compute_shader_handle;
    RETURN_IF_ERROR(context->deps.logger, ret,
                    renderer_create_shader(context->deps.renderer, BACKGROUND_COMPUTE_SHADER_U32_CODE, BACKGROUND_COMPUTE_SHADER_BYTES_LEN, &compute_shader_handle),
                    "Failed to create shader: %d", ret);

    RendererPushConstantsInfo push_constants_info[] = {{
        .render_stage_flags = RENDERER_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = sizeof(BackgroundPushConstants),
    }};

    RendererResourceSetLayoutHandle descriptor_set_handles[] = {context->draw_image_resource_set_layout_handle};
    RendererPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .push_constants_len = ARRAY_SIZE(push_constants_info),
        .push_constants = push_constants_info,
        .resource_set_layout_handles_len = ARRAY_SIZE(descriptor_set_handles),
        .resource_set_layout_handles = descriptor_set_handles,
    };

    RETURN_IF_ERROR(context->deps.logger, ret,
                    renderer_create_pipeline_layout(context->deps.renderer, &pipeline_layout_create_info, &context->gradient_pipeline_layout_handle),
                    "Failed to create pipeline layout: %d", ret);

    RendererComputePipelineCreateInfo pipeline_create_info = {
        .compute_shader_handle = compute_shader_handle,
        .compute_shader_entry_point = "main",
        .layout_handle = context->gradient_pipeline_layout_handle,
    };

    RETURN_IF_ERROR(context->deps.logger, ret,
                    renderer_create_compute_pipeline(context->deps.renderer, &pipeline_create_info, &context->background_pipeline_handle),
                    "Failed to create compute pipeline: %d", ret);

    RETURN_IF_ERROR(context->deps.logger, ret, renderer_destroy_shader(context->deps.renderer, compute_shader_handle),
                    "Failed to destroy shader: %d", ret);

    return 0;
}

int32_t draw_default_present(DrawContext *context)
{
    assert(context != NULL);

    int32_t ret;
    RendererCommandList *command_list;
    RendererInterface *renderer = context->deps.renderer;
    RETURN_IF_ERROR(context->deps.logger, ret, renderer_begin_frame(renderer, &command_list),
                    "Failed to begin frame: %d", ret);

    if (ret == 1 || ret == 2)
    {
        return ret;
    }

    RendererResourceSetHandle draw_image_resource_set_handle;
    RETURN_IF_ERROR(context->deps.logger, ret, renderer_allocate_transient_resource_set(renderer, context->draw_image_resource_set_layout_handle, &draw_image_resource_set_handle),
                    "Failed to allocate transient resource set: %d", ret);

    renderer_update_transient_resource_set(renderer, draw_image_resource_set_handle);

    BackgroundPushConstants push_constants = {
        .top_left = {1, 0, 0, 1},
        .top_right = {0, 0, 1, 1},
        .bottom_left = {0, 1, 1, 1},
        .bottom_right = {0, 1, 0, 1},
    };

    renderer_cmd_bind_compute_pipeline(context->deps.renderer, command_list, context->background_pipeline_handle);
    renderer_cmd_bind_resource_sets(context->deps.renderer, command_list, RENDERER_PIPELINE_TYPE_COMPUTE, context->gradient_pipeline_layout_handle, 0, 1, &draw_image_resource_set_handle, 0, NULL);
    renderer_cmd_push_constants(context->deps.renderer, command_list, context->gradient_pipeline_layout_handle, RENDERER_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(BackgroundPushConstants), &push_constants);

    uint32_t extent[2] = {0};
    renderer_dummy_get_extent(context->deps.renderer, extent);
    renderer_cmd_dispatch(context->deps.renderer, command_list, (uint32_t)ceil(extent[0] / 16.0), (uint32_t)ceil(extent[1] / 16.0), 1);

    // renderer_cmd_begin_render_pass(renderer, command_list);

    // renderer_cmd_bind_pipeline(renderer, command_list, pipeline_handle);
    // renderer_cmd_draw(renderer, command_list, 3, 1, 0, 0);

    // renderer_cmd_end_render_pass(renderer, command_list);

    RETURN_IF_ERROR(context->deps.logger, ret, renderer_end_frame(renderer),
                    "Failed to end frame: %d", ret);
    return 0;
}

void draw_default_on_window_resize(DrawContext *context, uint32_t width, uint32_t height)
{
    assert(context != NULL);

    renderer_on_window_resize(context->deps.renderer, width, height);
}