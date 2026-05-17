#include "draw_default_start.h"

#include <stdint.h>

#include <plugin_sdk/renderer/v1/renderer_interface.h>
#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(draw_default_start, LOG_LEVEL_DEBUG)

#include "shader_background_compute.h"
#include "shader_colored_triangle_vertex.h"
#include "shader_colored_triangle_fragment.h"

#include "draw_default_register.h"

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

    return 0;
}

int32_t draw_default_create_draw_image(DrawContext *context)
{
    assert(context != NULL);

    int32_t ret;
    LoggerInterface *logger = context->deps.logger;
    RendererInterface *renderer = context->deps.renderer;

    RendererImageProperties render_image_properties;
    RETURN_IF_ERROR(logger, ret,
                    renderer_get_image_properties(
                        renderer,
                        renderer_get_render_image_handle(renderer),
                        &render_image_properties),
                    "Failed to get render image properties: %d", ret);

    RendererImageCreateInfo renderer_image_create_info = {
        .extent = render_image_properties.extent,
        .format = RENDERER_IMAGE_FORMAT_R16G16B16A16_SFLOAT,
        .usage_flags = RENDERER_IMAGE_USAGE_TRANSFER_SRC_BIT | RENDERER_IMAGE_USAGE_TRANSFER_DST_BIT | RENDERER_IMAGE_USAGE_STORAGE_BIT | RENDERER_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .memory_usage = RENDERER_IMAGE_MEMORY_USAGE_GPU_ONLY,
    };

    RETURN_IF_ERROR(logger, ret, renderer_create_image(renderer, &renderer_image_create_info, &context->draw_image_handle),
                    "Failed to create draw image: %d", ret);

    context->draw_extent.width = render_image_properties.extent.width;
    context->draw_extent.height = render_image_properties.extent.height;

    return 0;
}

int32_t create_triangle_pipeline(DrawContext *context)
{
    assert(context != NULL);
    int32_t ret;

    LoggerInterface *logger = context->deps.logger;
    RendererInterface *renderer = context->deps.renderer;

    RendererShaderHandle vertex_shader_handle;
    RendererShaderHandle fragment_shader_handle;
    RETURN_IF_ERROR(logger, ret,
                    renderer_create_shader(renderer, COLORED_TRIANGLE_VERTEX_SHADER_U32_CODE, COLORED_TRIANGLE_VERTEX_SHADER_BYTES_LEN, &vertex_shader_handle),
                    "Failed to create shader: %d", ret);

    RETURN_IF_ERROR(logger, ret,
                    renderer_create_shader(renderer, COLORED_TRIANGLE_FRAGMENT_SHADER_U32_CODE, COLORED_TRIANGLE_FRAGMENT_SHADER_BYTES_LEN, &fragment_shader_handle),
                    "Failed to create shader: %d", ret);

    RendererPipelineLayoutCreateInfo pipeline_layout_create_info = {0};
    RETURN_IF_ERROR(logger, ret, renderer_create_pipeline_layout(renderer, &pipeline_layout_create_info, &context->triangle_pipeline_layout_handle),
                    "Failed to create triangle pipeline layout: %d", ret);

    RendererImageProperties draw_image_properties;
    RETURN_IF_ERROR(logger, ret, renderer_get_image_properties(renderer, context->draw_image_handle, &draw_image_properties),
                    "Failed to get draw image properties: %d", ret);

    RendererGraphicsPipelineCreateInfo pipeline_create_info = {
        .layout_handle = context->triangle_pipeline_layout_handle,

        .vertex_shader = {
            .shader_handle = vertex_shader_handle,
            .entry_point = "main",
        },
        .fragment_shader = {
            .shader_handle = fragment_shader_handle,
            .entry_point = "main",
        },

        .color_attachment_format = draw_image_properties.format,
        .depth_attachment_format = RENDERER_IMAGE_FORMAT_UNDEFINED,
        .topology = RENDERER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .fill_mode = RENDERER_FILL_MODE_SOLID,
        .cull_mode = RENDERER_CULL_MODE_NONE,
        .front_face = RENDERER_FRONT_FACE_CLOCKWISE,
        .blend_mode = RENDERER_BLEND_MODE_NONE,
    };
    RETURN_IF_ERROR(logger, ret, renderer_create_graphics_pipeline(renderer, &pipeline_create_info, &context->triangle_pipeline_handle),
                    "Failed to create triangle pipeline: %d", ret);

    RETURN_IF_ERROR(logger, ret, renderer_destroy_shader(renderer, fragment_shader_handle),
                    "Failed to destroy fragment shader: %d", ret);
    RETURN_IF_ERROR(logger, ret, renderer_destroy_shader(renderer, vertex_shader_handle),
                    "Failed to destroy vertex shader: %d", ret);

    return 0;
}

int32_t create_background_pipeline(DrawContext *context)
{
    assert(context != NULL);
    int32_t ret;

    LoggerInterface *logger = context->deps.logger;
    RendererInterface *renderer = context->deps.renderer;

    RendererShaderHandle compute_shader_handle;
    RETURN_IF_ERROR(logger, ret,
                    renderer_create_shader(renderer, BACKGROUND_COMPUTE_SHADER_U32_CODE, BACKGROUND_COMPUTE_SHADER_BYTES_LEN, &compute_shader_handle),
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

    RETURN_IF_ERROR(logger, ret,
                    renderer_create_pipeline_layout(renderer, &pipeline_layout_create_info, &context->background_pipeline_layout_handle),
                    "Failed to create pipeline layout: %d", ret);

    RendererComputePipelineCreateInfo pipeline_create_info = {
        .compute_shader = {
            .shader_handle = compute_shader_handle,
            .entry_point = "main",
        },
        .layout_handle = context->background_pipeline_layout_handle,
    };

    RETURN_IF_ERROR(logger, ret,
                    renderer_create_compute_pipeline(renderer, &pipeline_create_info, &context->background_pipeline_handle),
                    "Failed to create compute pipeline: %d", ret);

    RETURN_IF_ERROR(logger, ret, renderer_destroy_shader(renderer, compute_shader_handle),
                    "Failed to destroy shader: %d", ret);

    return 0;
}

int32_t draw_default_start(DrawContext *context)
{
    assert(context != NULL);
    int32_t ret;

    LoggerInterface *logger = context->deps.logger;
    RendererInterface *renderer = context->deps.renderer;

    RETURN_IF_ERROR(logger, ret, renderer_start(renderer),
                    "Error initializing renderer: %d", ret);

    RETURN_IF_ERROR(logger, ret, draw_default_create_draw_image(context), "Failed to create draw image: %d", ret);

    RETURN_IF_ERROR(logger, ret, create_resource_set_layouts(context),
                    "Failed to create resource set layouts: %d", ret);

    RETURN_IF_ERROR(logger, ret, create_background_pipeline(context),
                    "Failed to create background pipeline: %d", ret);

    RETURN_IF_ERROR(logger, ret, create_triangle_pipeline(context),
                    "Failed to create triangle pipeline: %d", ret);

    return 0;
}

void draw_default_cleanup(DrawContext *context)
{
    assert(context != NULL);

    RendererInterface *renderer = context->deps.renderer;

    (void)renderer_destroy_graphics_pipeline(renderer, context->triangle_pipeline_handle);
    (void)renderer_destroy_pipeline_layout(renderer, context->triangle_pipeline_layout_handle);
    (void)renderer_destroy_compute_pipeline(renderer, context->background_pipeline_handle);
    (void)renderer_destroy_pipeline_layout(renderer, context->background_pipeline_layout_handle);
    (void)renderer_destroy_resource_set_layout(renderer, context->draw_image_resource_set_layout_handle);
    (void)renderer_destroy_image(renderer, context->draw_image_handle);
}