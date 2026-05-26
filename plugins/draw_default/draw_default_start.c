#include "draw_default_start.h"

#include <stdint.h>

#include <plugin_sdk/renderer/v1/renderer_interface.h>
#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(draw_default_start, LOG_LEVEL_DEBUG)

#include "shader_background_compute.h"
#include "shader_colored_triangle_vertex.h"
#include "shader_colored_triangle_mesh_vertex.h"
#include "shader_colored_triangle_fragment.h"

#include "draw_default_register.h"

typedef struct DD_Shaders
{
    RendererShaderCreateInfo triangle_vertex_shader;
    RendererShaderCreateInfo triangle_mesh_vertex_shader;
    RendererShaderCreateInfo triangle_fragment_shader;
    RendererShaderCreateInfo background_compute_shader;
} DD_Shaders;

int32_t create_shaders(DrawContext *context, DD_Shaders *out_shaders)
{
    assert(context != NULL);
    assert(out_shaders != NULL);

    int32_t ret;
    LoggerInterface *logger = context->deps.logger;
    RendererInterface *renderer = context->deps.renderer;

    RETURN_IF_ERROR(logger, ret,
                    renderer_create_shader(renderer, COLORED_TRIANGLE_VERTEX_SHADER_U32_CODE, COLORED_TRIANGLE_VERTEX_SHADER_BYTES_LEN, &out_shaders->triangle_vertex_shader.shader_handle),
                    "Failed to triangle vertex create shader: %d", ret);
    out_shaders->triangle_vertex_shader.entry_point = "main";

    RETURN_IF_ERROR(logger, ret,
                    renderer_create_shader(renderer, COLORED_TRIANGLE_MESH_VERTEX_SHADER_U32_CODE, COLORED_TRIANGLE_MESH_VERTEX_SHADER_BYTES_LEN, &out_shaders->triangle_mesh_vertex_shader.shader_handle),
                    "Failed to triangle vertex create shader: %d", ret);
    out_shaders->triangle_mesh_vertex_shader.entry_point = "main";

    RETURN_IF_ERROR(logger, ret,
                    renderer_create_shader(renderer, COLORED_TRIANGLE_FRAGMENT_SHADER_U32_CODE, COLORED_TRIANGLE_FRAGMENT_SHADER_BYTES_LEN, &out_shaders->triangle_fragment_shader.shader_handle),
                    "Failed to triangle fragment create shader: %d", ret);
    out_shaders->triangle_fragment_shader.entry_point = "main";

    RETURN_IF_ERROR(logger, ret,
                    renderer_create_shader(renderer, BACKGROUND_COMPUTE_SHADER_U32_CODE, BACKGROUND_COMPUTE_SHADER_BYTES_LEN, &out_shaders->background_compute_shader.shader_handle),
                    "Failed to create shader: %d", ret);
    out_shaders->background_compute_shader.entry_point = "main";

    return 0;
}

void destroy_shaders(DrawContext *context, DD_Shaders *shaders)
{
    assert(context != NULL);
    assert(shaders != NULL);

    renderer_destroy_shader(context->deps.renderer, shaders->triangle_vertex_shader.shader_handle);
    renderer_destroy_shader(context->deps.renderer, shaders->triangle_mesh_vertex_shader.shader_handle);
    renderer_destroy_shader(context->deps.renderer, shaders->triangle_fragment_shader.shader_handle);
    renderer_destroy_shader(context->deps.renderer, shaders->background_compute_shader.shader_handle);
}

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
        .memory_usage = RENDERER_MEMORY_USAGE_GPU_ONLY,
    };

    RETURN_IF_ERROR(logger, ret, renderer_create_image(renderer, &renderer_image_create_info, &context->draw_image_handle),
                    "Failed to create draw image: %d", ret);

    context->draw_extent.width = render_image_properties.extent.width;
    context->draw_extent.height = render_image_properties.extent.height;

    return 0;
}

int32_t create_triangle_pipeline(DrawContext *context, DD_Shaders *shaders)
{
    assert(context != NULL);
    int32_t ret;

    LoggerInterface *logger = context->deps.logger;
    RendererInterface *renderer = context->deps.renderer;

    RendererPipelineLayoutCreateInfo pipeline_layout_create_info = {0};
    RETURN_IF_ERROR(logger, ret, renderer_create_pipeline_layout(renderer, &pipeline_layout_create_info, &context->triangle_pipeline_layout_handle),
                    "Failed to create triangle pipeline layout: %d", ret);

    RendererImageProperties draw_image_properties;
    RETURN_IF_ERROR(logger, ret, renderer_get_image_properties(renderer, context->draw_image_handle, &draw_image_properties),
                    "Failed to get draw image properties: %d", ret);

    RendererGraphicsPipelineCreateInfo pipeline_create_info = {
        .layout_handle = context->triangle_pipeline_layout_handle,

        .vertex_shader = shaders->triangle_vertex_shader,
        .fragment_shader = shaders->triangle_fragment_shader,

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

    return 0;
}

int32_t create_triangle_mesh_pipeline(DrawContext *context, DD_Shaders *shaders)
{
    assert(context != NULL);
    assert(shaders != NULL);

    int32_t ret;

    LoggerInterface *logger = context->deps.logger;
    RendererInterface *renderer = context->deps.renderer;

    RendererPushConstantsInfo gpu_push_constants_info = {
        .offset = 0,
        .size = sizeof(GPUDrawPushConstants),
        .render_stage_flags = RENDERER_SHADER_STAGE_VERTEX_BIT,
    };

    RendererPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .push_constants_len = 1,
        .push_constants = &gpu_push_constants_info,
    };

    RETURN_IF_ERROR(logger, ret, renderer_create_pipeline_layout(renderer, &pipeline_layout_create_info, &context->triangle_mesh_pipeline_layout_handle),
                    "Failed to create triangle mesh pipeline layout: %d", ret);

    RendererImageProperties draw_image_properties;
    RETURN_IF_ERROR(logger, ret, renderer_get_image_properties(renderer, context->draw_image_handle, &draw_image_properties),
                    "Failed to get draw image properties: %d", ret);

    RendererGraphicsPipelineCreateInfo pipeline_create_info = {
        .layout_handle = context->triangle_mesh_pipeline_layout_handle,

        .vertex_shader = shaders->triangle_mesh_vertex_shader,
        .fragment_shader = shaders->triangle_fragment_shader,

        .color_attachment_format = draw_image_properties.format,
        .depth_attachment_format = RENDERER_IMAGE_FORMAT_UNDEFINED,
        .topology = RENDERER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .fill_mode = RENDERER_FILL_MODE_SOLID,
        .cull_mode = RENDERER_CULL_MODE_NONE,
        .front_face = RENDERER_FRONT_FACE_CLOCKWISE,
        .blend_mode = RENDERER_BLEND_MODE_NONE,
    };

    RETURN_IF_ERROR(logger, ret, renderer_create_graphics_pipeline(renderer, &pipeline_create_info, &context->triangle_mesh_pipeline_handle),
                    "Failed to create triangle mesh pipeline: %d", ret);

    return 0;
}

int32_t create_background_pipeline(DrawContext *context, DD_Shaders *shaders)
{
    assert(context != NULL);
    int32_t ret;

    LoggerInterface *logger = context->deps.logger;
    RendererInterface *renderer = context->deps.renderer;

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
        .compute_shader = shaders->background_compute_shader,
        .layout_handle = context->background_pipeline_layout_handle,
    };

    RETURN_IF_ERROR(logger, ret,
                    renderer_create_compute_pipeline(renderer, &pipeline_create_info, &context->background_pipeline_handle),
                    "Failed to create compute pipeline: %d", ret);

    return 0;
}

typedef struct UploadVertexIndexBuffersInfo
{
    LoggerInterface *logger;
    RendererInterface *renderer;
    RendererUploadBufferDataInfo *vertex_upload_buffer_data_info;
    RendererUploadBufferDataInfo *index_upload_buffer_data_info;
} UploadVertexIndexBuffersInfo;

int32_t upload_vertex_index_buffers(RendererCommandList *command_list, void *user_data)
{
    assert(command_list != NULL);
    assert(user_data != NULL);

    UploadVertexIndexBuffersInfo *upload_vertex_index_buffers_info = (UploadVertexIndexBuffersInfo *)user_data;

    int32_t ret;
    LoggerInterface *logger = upload_vertex_index_buffers_info->logger;
    RendererInterface *renderer = upload_vertex_index_buffers_info->renderer;

    RETURN_IF_ERROR(logger, ret, renderer_upload_buffer_data(renderer, command_list, upload_vertex_index_buffers_info->vertex_upload_buffer_data_info),
                    "Failed to upload vertex buffer data: %d", ret);

    RETURN_IF_ERROR(logger, ret, renderer_upload_buffer_data(renderer, command_list, upload_vertex_index_buffers_info->index_upload_buffer_data_info),
                    "Failed to upload index buffer data: %d", ret);

    return 0;
}

int32_t create_mesh_buffers(DrawContext *context)
{
    assert(context != NULL);
    int32_t ret;

    LoggerInterface *logger = context->deps.logger;
    RendererInterface *renderer = context->deps.renderer;

    CREATE_INITIALIZED_ARRAY(
    Vertex, main_mesh_vertices_a,
    {
        (Vertex){ .position = { 0.0000f,  -0.5000f, 0.0f }, .color = { 0.051f, 0.463f, 0.878f, 1.0f } }, // top tip
        (Vertex){ .position = { 0.1123f,  -0.1545f, 0.0f }, .color = { 1.00f, 0.62f, 0.18f, 1.0f } },
        (Vertex){ .position = { 0.4755f,  -0.1545f, 0.0f }, .color = { 1.00f, 0.82f, 0.10f, 1.0f } },
        (Vertex){ .position = { 0.1816f, 0.0590f, 0.0f }, .color = { 0.95f, 0.42f, 0.22f, 1.0f } },
        (Vertex){ .position = { 0.2939f, 0.4045f, 0.0f }, .color = { 1.00f, 0.72f, 0.08f, 1.0f } },
        (Vertex){ .position = { 0.0000f, 0.1910f, 0.0f }, .color = { 0.90f, 0.28f, 0.35f, 1.0f } },
        (Vertex){ .position = {-0.2939f, 0.4045f, 0.0f }, .color = { 1.00f, 0.76f, 0.12f, 1.0f } },
        (Vertex){ .position = {-0.1816f, 0.0590f, 0.0f }, .color = { 0.98f, 0.48f, 0.20f, 1.0f } },
        (Vertex){ .position = {-0.4755f,  -0.1545f, 0.0f }, .color = { 1.00f, 0.84f, 0.14f, 1.0f } },
        (Vertex){ .position = {-0.1123f,  -0.1545f, 0.0f }, .color = { 1.00f, 0.58f, 0.24f, 1.0f } },
        (Vertex){ .position = { 0.0000f,  -0.0000f, 0.0f }, .color = { 0.82f, 0.22f, 0.58f, 1.0f } },
    });
   

    CREATE_INITIALIZED_ARRAY(
        uint32_t,
        main_mesh_indices_a,
        {
            0,
            1,
            9,
            2,
            3,
            1,
            4,
            5,
            3,
            6,
            7,
            5,
            8,
            9,
            7,
            10,
            9,
            1,
            10,
            1,
            3,
            10,
            3,
            5,
            10,
            5,
            7,
            10,
            7,
            9,
        });

    context->rect_mesh_buffers.indices_len = (uint32_t)GET_ARRAY_LENGTH(main_mesh_indices_a);

    const size_t vertex_buffer_size = GET_ARRAY_LENGTH(main_mesh_vertices_a) * sizeof(*main_mesh_vertices_a);
    const size_t index_buffer_size = GET_ARRAY_LENGTH(main_mesh_indices_a) * sizeof(*main_mesh_indices_a);

    RendererBufferCreateInfo vertex_buffer_create_info = {
        .memory_usage = RENDERER_MEMORY_USAGE_GPU_ONLY,
        .size = vertex_buffer_size,
        .usage_flags = RENDERER_BUFFER_USAGE_STORAGE_BUFFER_BIT | RENDERER_BUFFER_USAGE_TRANSFER_DST_BIT | RENDERER_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    };

    RETURN_IF_ERROR(logger, ret,
                    renderer_create_buffer(renderer, &vertex_buffer_create_info, &context->rect_mesh_buffers.vertex_buffer_handle),
                    "Unable to create vertex buffer: %d", ret);

    RETURN_IF_ERROR(logger, ret,
                    renderer_get_buffer_device_address(renderer, context->rect_mesh_buffers.vertex_buffer_handle, &context->rect_mesh_buffers.vertex_buffer_address),
                    "Failed to get buffer device address: %d", ret);

    RendererBufferCreateInfo index_buffer_create_info = {
        .memory_usage = RENDERER_MEMORY_USAGE_GPU_ONLY,
        .size = index_buffer_size,
        .usage_flags = RENDERER_BUFFER_USAGE_INDEX_BUFFER_BIT | RENDERER_BUFFER_USAGE_TRANSFER_DST_BIT,
    };

    RETURN_IF_ERROR(logger, ret,
                    renderer_create_buffer(renderer, &index_buffer_create_info, &context->rect_mesh_buffers.index_buffer_handle),
                    "Unable to create index buffer: %d", ret);

    RendererUploadBufferDataInfo vertex_upload_buffer_data = {
        .upload_size = vertex_buffer_size,
        .upload_data = main_mesh_vertices_a,
        .destination_offset = 0,
        .destination_buffer_handle = context->rect_mesh_buffers.vertex_buffer_handle,
    };
    RendererUploadBufferDataInfo index_upload_buffer_data = {
        .upload_size = index_buffer_size,
        .upload_data = main_mesh_indices_a,
        .destination_offset = 0,
        .destination_buffer_handle = context->rect_mesh_buffers.index_buffer_handle,
    };

    UploadVertexIndexBuffersInfo upload_vertex_index_buffers_info = {
        .logger = logger,
        .renderer = renderer,
        .vertex_upload_buffer_data_info = &vertex_upload_buffer_data,
        .index_upload_buffer_data_info = &index_upload_buffer_data,
    };

    RETURN_IF_ERROR(logger, ret,
                    renderer_immediate_execute(renderer, upload_vertex_index_buffers, &upload_vertex_index_buffers_info),
                    "Failed to upload vertex and index buffers: %d", ret);

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

    DD_Shaders shaders;
    RETURN_IF_ERROR(logger, ret, create_shaders(context, &shaders),
                    "Failed to create shaders: %d", ret);

    RETURN_IF_ERROR(logger, ret, create_background_pipeline(context, &shaders),
                    "Failed to create background pipeline: %d", ret);

    RETURN_IF_ERROR(logger, ret, create_triangle_pipeline(context, &shaders),
                    "Failed to create triangle pipeline: %d", ret);

    RETURN_IF_ERROR(logger, ret, create_triangle_mesh_pipeline(context, &shaders),
                    "Failed to create triangle pipeline: %d", ret);

    destroy_shaders(context, &shaders);

    RETURN_IF_ERROR(logger, ret, create_mesh_buffers(context),
                    "Failed to create mesh buffer: %d", ret);

    return 0;
}

void draw_default_cleanup(DrawContext *context)
{
    assert(context != NULL);

    int32_t ret;
    LoggerInterface *logger = context->deps.logger;
    RendererInterface *renderer = context->deps.renderer;

    ret = renderer_destroy_buffer(renderer, context->rect_mesh_buffers.index_buffer_handle);
    if (ret < 0)
    {
        LOG_WRN_TRACE(logger, "Failed to destroy index buffer");
    }

    ret = renderer_destroy_buffer(renderer, context->rect_mesh_buffers.vertex_buffer_handle);
    if (ret < 0)
    {
        LOG_WRN_TRACE(logger, "Failed to destroy vertex buffer");
    }

    ret = renderer_destroy_graphics_pipeline(renderer, context->triangle_mesh_pipeline_handle);
    if (ret < 0)
    {
        LOG_WRN_TRACE(logger, "Failed to destroy graphics_pipeline: %d", ret);
    }
    ret = renderer_destroy_pipeline_layout(renderer, context->triangle_mesh_pipeline_layout_handle);
    if (ret < 0)
    {
        LOG_WRN_TRACE(logger, "Failed to destroy pipeline_layout: %d", ret);
    }
    ret = renderer_destroy_graphics_pipeline(renderer, context->triangle_pipeline_handle);
    if (ret < 0)
    {
        LOG_WRN_TRACE(logger, "Failed to destroy graphics_pipeline: %d", ret);
    }
    ret = renderer_destroy_pipeline_layout(renderer, context->triangle_pipeline_layout_handle);
    if (ret < 0)
    {
        LOG_WRN_TRACE(logger, "Failed to destroy pipeline_layout: %d", ret);
    }
    ret = renderer_destroy_compute_pipeline(renderer, context->background_pipeline_handle);
    if (ret < 0)
    {
        LOG_WRN_TRACE(logger, "Failed to destroy compute_pipeline: %d", ret);
    }
    ret = renderer_destroy_pipeline_layout(renderer, context->background_pipeline_layout_handle);
    if (ret < 0)
    {
        LOG_WRN_TRACE(logger, "Failed to destroy pipeline_layout: %d", ret);
    }
    ret = renderer_destroy_resource_set_layout(renderer, context->draw_image_resource_set_layout_handle);
    if (ret < 0)
    {
        LOG_WRN_TRACE(logger, "Failed to destroy resource_set_layout: %d", ret);
    }
    ret = renderer_destroy_image(renderer, context->draw_image_handle);
    if (ret < 0)
    {
        LOG_WRN_TRACE(logger, "Failed to destroy image: %d", ret);
    }
}