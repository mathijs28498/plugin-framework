#include "draw_default_render_frame.h"

#include <stdint.h>
#include <math.h>

#include <plugin_sdk/renderer/v1/renderer_interface.h>
#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(draw_default_render_frame, LOG_LEVEL_DEBUG);

#include "draw_default_register.h"
#include "draw_default_start.h"
#include "draw_default.h"

int32_t draw_default_render_frame(DrawContext *context, RendererCommandList *command_list)
{
    assert(context != NULL);
    assert(command_list != NULL);

    int32_t ret;
    LoggerInterface *logger = context->deps.logger;
    RendererInterface *renderer = context->deps.renderer;

    if (renderer_consume_has_resized(renderer))
    {
        RETURN_IF_ERROR(logger, ret, renderer_destroy_image(renderer, context->draw_image_handle),
                        "Failed to destroy draw image: %d", ret);
        RETURN_IF_ERROR(logger, ret, draw_default_create_draw_image(context),
                        "Failed to create draw image: %d", ret);
    }

    // Get the render image information
    RendererImageHandle render_image_handle = renderer_get_render_image_handle(renderer);
    RendererImageProperties render_image_properties;
    RETURN_IF_ERROR(logger, ret, renderer_get_image_properties(renderer, render_image_handle, &render_image_properties),
                    "Failed to get render image properties: %d", ret);
    RendererExtent2D render_extent_2d = {
        .width = render_image_properties.extent.width,
        .height = render_image_properties.extent.height,
    };

    // Start background pass
    renderer_cmd_transition_image(renderer, command_list, context->draw_image_handle, RENDERER_IMAGE_LAYOUT_UNDEFINED, RENDERER_IMAGE_LAYOUT_GENERAL);

    RendererResourceSetHandle draw_image_resource_set_handle;
    RETURN_IF_ERROR(logger, ret, renderer_allocate_transient_resource_set(renderer, context->draw_image_resource_set_layout_handle, &draw_image_resource_set_handle),
                    "Failed to allocate transient resource set: %d", ret);

    RendererResourceImageBinding draw_image_binding = {
        .image_handle = context->draw_image_handle,
        .image_layout = RENDERER_IMAGE_LAYOUT_GENERAL,
    };
    RendererResourceSetWrite draw_image_resource_set_write = {
        .binding = 0,
        .resource_type = RENDERER_RESOURCE_TYPE_STORAGE_IMAGE,
        .resource_bindings_len = 1,
        .image_bindings = &draw_image_binding,
    };

    RendererResourceSetUpdateInfo resource_set_update_info = {
        .resource_set_handle = draw_image_resource_set_handle,
        .resource_set_writes_len = 1,
        .resource_set_writes = &draw_image_resource_set_write,
    };

    renderer_update_resource_set(renderer, &resource_set_update_info);

    BackgroundPushConstants push_constants = {
        .top_left = {1, 0, 0, 1},
        .top_right = {0, 0, 1, 1},
        .bottom_left = {0, 1, 1, 1},
        .bottom_right = {0, 1, 0, 1},
    };

    renderer_cmd_bind_compute_pipeline(renderer, command_list, context->background_pipeline_handle);
    renderer_cmd_bind_resource_sets(renderer, command_list, RENDERER_PIPELINE_TYPE_COMPUTE, context->background_pipeline_layout_handle, 0, 1, &draw_image_resource_set_handle, 0, NULL);
    renderer_cmd_push_constants(renderer, command_list, context->background_pipeline_layout_handle, RENDERER_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(BackgroundPushConstants), &push_constants);

    renderer_cmd_dispatch(renderer, command_list, (uint32_t)ceil(render_extent_2d.width / 16.0), (uint32_t)ceil(render_extent_2d.height / 16.0), 1);

    renderer_cmd_transition_image(renderer, command_list, context->draw_image_handle, RENDERER_IMAGE_LAYOUT_GENERAL, RENDERER_IMAGE_LAYOUT_COLOR_ATTACHMENT);

    // Start graphics rendering
    RendererBeginRenderingInfo begin_rendering_info = {
        .color_attachment_info = {
            .image_handle = context->draw_image_handle,
            .load_op = RENDERER_ATTACHMENT_LOAD_OP_LOAD,
            .store_op = RENDERER_ATTACHMENT_STORE_OP_STORE,
        },
        .depth_attachment_info = NULL,
    };
    renderer_cmd_begin_rendering(renderer, command_list, &begin_rendering_info);
    renderer_cmd_set_viewport(renderer, command_list, render_extent_2d);
    renderer_cmd_set_scissor(renderer, command_list, render_extent_2d);

    renderer_cmd_bind_graphics_pipeline(renderer, command_list, context->triangle_pipeline_handle);
    renderer_cmd_draw(renderer, command_list, 3, 1, 0, 0);

    // End graphics rendering
    renderer_cmd_end_rendering(renderer, command_list);

    // End frame
    renderer_cmd_transition_image(renderer, command_list, context->draw_image_handle, RENDERER_IMAGE_LAYOUT_COLOR_ATTACHMENT, RENDERER_IMAGE_LAYOUT_TRANSFER_SRC);
    renderer_cmd_transition_image(renderer, command_list, render_image_handle, RENDERER_IMAGE_LAYOUT_UNDEFINED, RENDERER_IMAGE_LAYOUT_TRANSFER_DST);
    renderer_cmd_blit_image_to_image(renderer, command_list, context->draw_image_handle, render_image_handle,
                                     context->draw_extent, render_extent_2d);
    return 0;
}