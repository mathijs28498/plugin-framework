#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../../plugin_utils.h"
#include "../../plugin_macros.h"
#include "renderer_types.h"

#pragma pack(push, 8)

typedef struct RendererContext RendererContext;

typedef struct RendererWindowConfig
{
    void *window_handle;
    void *display_handle;

    uint32_t width;
    uint32_t height;
    bool enable_vsync;
} RendererWindowConfig;

typedef enum RendererImageUsageBits
{
    RENDERER_IMAGE_USAGE_TRANSFER_SRC_BIT = 0x00000001,
    RENDERER_IMAGE_USAGE_TRANSFER_DST_BIT = 0x00000002,
    RENDERER_IMAGE_USAGE_SAMPLED_BIT = 0x00000004,
    RENDERER_IMAGE_USAGE_STORAGE_BIT = 0x00000008,
    RENDERER_IMAGE_USAGE_COLOR_ATTACHMENT_BIT = 0x00000010,
    RENDERER_IMAGE_USAGE_DEPTH_ATTACHMENT_BIT = 0x00000020,
} RendererImageUsageBits;

typedef uint32_t RendererImageUsageFlags;

typedef enum RendererShaderStageBits
{
    RENDERER_SHADER_STAGE_VERTEX_BIT = 0x00000001,
    RENDERER_SHADER_STAGE_FRAGMENT_BIT = 0x00000002,
    RENDERER_SHADER_STAGE_COMPUTE_BIT = 0x00000004,
} RendererShaderStageBits;

typedef uint32_t RendererShaderStageFlags;

typedef enum RendererImageLayout
{
    RENDERER_IMAGE_LAYOUT_UNDEFINED = 0,
    RENDERER_IMAGE_LAYOUT_GENERAL = 1,
    RENDERER_IMAGE_LAYOUT_COLOR_ATTACHMENT = 2,
    RENDERER_IMAGE_LAYOUT_TRANSFER_SRC = 6,
    RENDERER_IMAGE_LAYOUT_TRANSFER_DST = 7,
    RENDERER_IMAGE_LAYOUT_PRESENT_SRC = 1000001002,
} RendererImageLayout;

typedef enum RendererImageFormat
{
    RENDERER_IMAGE_FORMAT_UNDEFINED,
    RENDERER_IMAGE_FORMAT_R8G8B8A8_UNORM,
    RENDERER_IMAGE_FORMAT_R8G8B8A8_SRGB,
    RENDERER_IMAGE_FORMAT_R16G16B16A16_SFLOAT,
    RENDERER_IMAGE_FORMAT_R32G32B32A32_SFLOAT,
    RENDERER_IMAGE_FORMAT_D32_SFLOAT,
    RENDERER_IMAGE_FORMAT_D24_UNORM_S8_UINT,
} RendererImageFormat;

typedef enum RendererImageMemoryUsage
{
    RENDERER_IMAGE_MEMORY_USAGE_GPU_ONLY,
} RendererImageMemoryUsage;

typedef struct RendererImageCreateInfo
{
    RendererImageFormat format;
    RendererImageUsageFlags usage_flags;
    RendererImageMemoryUsage memory_usage;
    RendererExtent3D extent;
} RendererImageCreateInfo;

typedef struct RendererImageProperties
{
    RendererImageFormat format;
    RendererExtent3D extent;
} RendererImageProperties;

typedef enum RendererPipelineType
{
    RENDERER_PIPELINE_TYPE_GRAPHICS,
    RENDERER_PIPELINE_TYPE_COMPUTE,
} RendererPipelineType;

typedef enum RendererResourceType
{
    RENDERER_RESOURCE_TYPE_SAMPLER = 0,
    RENDERER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER = 1,
    RENDERER_RESOURCE_TYPE_SAMPLED_IMAGE = 2,
    RENDERER_RESOURCE_TYPE_STORAGE_IMAGE = 3,
} RendererResourceType;

typedef struct RendererResourceSetLayoutBinding
{
    uint32_t binding;
    RendererResourceType resource_type;
    uint32_t resource_len;
    RendererShaderStageFlags stage_flags;
} RendererResourceSetLayoutBinding;

typedef struct RendererResourceSetLayoutCreateInfo
{
    uint32_t bindings_len;
    RendererResourceSetLayoutBinding *bindings;
} RendererResourceSetLayoutCreateInfo;

typedef struct RendererPushConstantsInfo
{
    RendererShaderStageFlags render_stage_flags;
    uint32_t offset;
    uint32_t size;
} RendererPushConstantsInfo;

TODO("Allow for multiple push constants")
TODO("Make this create info more complete with handles to resource sets")
typedef struct RendererPipelineLayoutCreateInfo
{
    uint32_t push_constants_len;
    RendererPushConstantsInfo *push_constants;
    uint32_t resource_set_layout_handles_len;
    RendererResourceSetLayoutHandle *resource_set_layout_handles;
} RendererPipelineLayoutCreateInfo;

typedef struct RendererResourceImageBinding
{
    RendererImageHandle image_handle;
    RendererImageLayout image_layout;
} RendererResourceImageBinding;

typedef struct RendererResourceSamplerBinding
{
    uint64_t sampler_handle;
} RendererResourceSamplerBinding;

typedef struct RendererResourceCombinedImageSamplerBinding
{
    RendererImageHandle image_handle;
    RendererImageLayout image_layout;
    RendererSamplerHandle sampler_handle;
} RendererResourceCombinedImageSamplerBinding;

typedef struct RendererResourceBufferBinding
{
    RendererBufferHandle buffer_handle;
    uint64_t offset;
    uint64_t size;
} RendererResourceBufferBinding;

typedef struct RendererResourceSetWrite
{
    uint32_t binding;
    uint32_t first_resource;

    RendererResourceType resource_type;
    uint32_t resource_bindings_len;
    union
    {
        const RendererResourceImageBinding *image_bindings;
        const RendererResourceSamplerBinding *sampler_bindings;
        const RendererResourceCombinedImageSamplerBinding *combined_image_sampler_bindings;
        const RendererResourceBufferBinding *buffer_bindings;
    };
} RendererResourceSetWrite;

typedef struct RendererResourceSetUpdateInfo
{
    RendererResourceSetHandle resource_set_handle;
    uint32_t resource_set_writes_len;
    const RendererResourceSetWrite *resource_set_writes;
} RendererResourceSetUpdateInfo;

typedef struct RendererShaderCreateInfo
{
    RendererShaderHandle shader_handle;
    const char *entry_point;
} RendererShaderCreateInfo;

typedef enum RendererPrimitiveTopology
{
    RENDERER_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
    RENDERER_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
    RENDERER_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
    RENDERER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
    RENDERER_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
    RENDERER_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN = 6,
} RendererPrimitiveTopology;

typedef enum RendererFillMode
{
    RENDERER_FILL_MODE_SOLID = 0,
    RENDERER_FILL_MODE_WIREFRAME = 1,
    RENDERER_FILL_MODE_POINT = 2,
} RendererFillMode;

typedef enum RendererCullModeFlagBits
{
    RENDERER_CULL_MODE_NONE = 0,
    RENDERER_CULL_MODE_FRONT_BIT = 0x00000001,
    RENDERER_CULL_MODE_BACK_BIT = 0x00000002,
    RENDERER_CULL_MODE_FRONT_AND_BACK = 0x00000003,
} RendererCullModeFlagBits;

typedef enum RendererFrontFace
{
    RENDERER_FRONT_FACE_COUNTER_CLOCKWISE = 0,
    RENDERER_FRONT_FACE_CLOCKWISE = 1,
} RendererFrontFace;

typedef enum RendererBlendMode
{
    RENDERER_BLEND_MODE_NONE = 0,
    RENDERER_BLEND_MODE_ADDITIVE = 1,
    RENDERER_BLEND_MODE_ALPHA_BLEND = 2,
} RendererBlendMode;

typedef struct RendererGraphicsPipelineCreateInfo
{
    RendererPipelineLayoutHandle layout_handle;

    RendererShaderCreateInfo vertex_shader;
    RendererShaderCreateInfo fragment_shader;

    RendererImageFormat color_attachment_format;
    RendererImageFormat depth_attachment_format;
    RendererPrimitiveTopology topology;
    RendererFillMode fill_mode;
    RendererCullModeFlagBits cull_mode;
    RendererFrontFace front_face;
    RendererBlendMode blend_mode;

} RendererGraphicsPipelineCreateInfo;

typedef struct RendererComputePipelineCreateInfo
{
    RendererShaderCreateInfo compute_shader;

    RendererPipelineLayoutHandle layout_handle;
} RendererComputePipelineCreateInfo;

typedef enum RendererAttachmentLoadOp
{
    RENDERER_ATTACHMENT_LOAD_OP_LOAD,
    RENDERER_ATTACHMENT_LOAD_OP_CLEAR,
    RENDERER_ATTACHMENT_LOAD_OP_DONT_CARE,
} RendererAttachmentLoadOp;

typedef enum RendererAttachmentStoreOp
{
    RENDERER_ATTACHMENT_STORE_OP_STORE,
    RENDERER_ATTACHMENT_STORE_OP_DONT_CARE,
} RendererAttachmentStoreOp;

typedef struct RendererClearColorValue
{
    float r, g, b, a;
} RendererClearColorValue;

typedef struct RendererClearValue
{
    union
    {
        RendererClearColorValue color;
        float depth;
    };
} RendererClearValue;

typedef struct RendererAttachmentInfo
{
    RendererImageHandle image_handle;
    RendererAttachmentLoadOp load_op;
    RendererAttachmentStoreOp store_op;
    RendererClearValue clear_value;
} RendererAttachmentInfo;

typedef struct RendererBeginRenderingInfo
{
    RendererAttachmentInfo color_attachment_info;
    RendererAttachmentInfo *depth_attachment_info;
} RendererBeginRenderingInfo;

typedef struct RendererVtable
{
    int32_t (*start)(RendererContext *context);
    int32_t (*begin_frame)(RendererContext *context, RendererCommandList **out_command_list);
    int32_t (*end_frame)(RendererContext *context);
    void (*on_window_resize)(RendererContext *context, uint32_t width, uint32_t height);
    bool (*consume_has_resized)(RendererContext *context);

    int32_t (*dummy_exec_fn)(RendererCommandList *command_list, void *user_data);

    int32_t (*immediate_execute)(RendererContext *context, ImmediateExecute_Fn immediate_execute_fn, void *user_data);

    RendererImageHandle (*get_render_image_handle)(RendererContext *context);
    int32_t (*get_image_properties)(RendererContext *context, RendererImageHandle image_handle, RendererImageProperties *out_image_properties);

    int32_t (*create_shader)(RendererContext *context, const uint32_t *shader_code_u32, size_t shader_code_bytes_len, RendererShaderHandle *out_shader_handle);
    int32_t (*destroy_shader)(RendererContext *context, RendererShaderHandle shader_handle);

    int32_t (*create_image)(RendererContext *context, RendererImageCreateInfo *renderer_image_create_info, RendererImageHandle *out_image_handle);
    int32_t (*destroy_image)(RendererContext *context, RendererImageHandle image_handle);
    int32_t (*create_resource_set_layout)(RendererContext *context, const RendererResourceSetLayoutCreateInfo *renderer_resource_set_layout_create_info, RendererResourceSetLayoutHandle *out_resource_set_layout_handle);
    int32_t (*destroy_resource_set_layout)(RendererContext *context, RendererResourceSetLayoutHandle resource_set_layout_handle);
    int32_t (*allocate_transient_resource_set)(RendererContext *context, RendererResourceSetLayoutHandle resource_set_layout_handle, RendererResourceSetHandle *out_resource_set_handle);
    void (*update_resource_set)(RendererContext *context, const RendererResourceSetUpdateInfo *resource_set_update_info);

    int32_t (*create_pipeline_layout)(RendererContext *context, const RendererPipelineLayoutCreateInfo *renderer_pipeline_layout_create_info, RendererPipelineLayoutHandle *out_pipeline_layout_handle);
    int32_t (*destroy_pipeline_layout)(RendererContext *context, RendererPipelineLayoutHandle pipeline_layout_handle);

    int32_t (*create_graphics_pipeline)(RendererContext *context, const RendererGraphicsPipelineCreateInfo *pipeline_create_info, RendererGraphicsPipelineHandle *out_pipeline_handle);
    int32_t (*create_compute_pipeline)(RendererContext *context, const RendererComputePipelineCreateInfo *pipeline_create_info, RendererComputePipelineHandle *out_pipeline_handle);
    int32_t (*destroy_graphics_pipeline)(RendererContext *context, RendererGraphicsPipelineHandle pipeline_handle);
    int32_t (*destroy_compute_pipeline)(RendererContext *context, RendererComputePipelineHandle pipeline_handle);

    void (*cmd_begin_rendering)(RendererContext *context, RendererCommandList *command_list, const RendererBeginRenderingInfo *renderer_begin_rendering_info);
    void (*cmd_end_rendering)(RendererContext *context, RendererCommandList *command_list);

    void (*cmd_set_viewport)(RendererContext *context, RendererCommandList *command_list, RendererExtent2D extent);
    void (*cmd_set_scissor)(RendererContext *context, RendererCommandList *command_list, RendererExtent2D extent);

    void (*cmd_bind_graphics_pipeline)(RendererContext *context, RendererCommandList *command_list, RendererGraphicsPipelineHandle pipeline_handle);
    void (*cmd_bind_compute_pipeline)(RendererContext *context, RendererCommandList *command_list, RendererComputePipelineHandle pipeline_handle);
    void (*cmd_draw)(RendererContext *context, RendererCommandList *command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
    void (*cmd_bind_resource_sets)(RendererContext *context, RendererCommandList *command_list, RendererPipelineType renderer_pipeline_type, RendererPipelineLayoutHandle pipeline_layout_handle, uint32_t first_set, uint32_t resource_set_len, const RendererResourceSetHandle *resource_set_handle, uint32_t dynamic_offset_len, const uint32_t *dynamic_offsets);

    void (*cmd_push_constants)(RendererContext *context, RendererCommandList *command_list, RendererPipelineLayoutHandle pipeline_layout_handle, RendererShaderStageFlags shader_stage_flags, uint32_t offset, uint32_t push_constants_size, void *push_constants);
    void (*cmd_dispatch)(RendererContext *context, RendererCommandList *command_list, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);

    void (*cmd_transition_image)(RendererContext *context, RendererCommandList *command_list, RendererImageHandle image_handle, RendererImageLayout renderer_current_layout, RendererImageLayout renderer_new_layout);
    void (*cmd_blit_image_to_image)(RendererContext *context, RendererCommandList *command_list, RendererImageHandle image_handle_source, RendererImageHandle image_handle_destination, RendererExtent2D extent_source, RendererExtent2D extent_destination);
} RendererVtable;

typedef struct RendererInterface
{
    RendererContext *context;
    RendererVtable *vtable;
} RendererInterface;

static inline int32_t renderer_start(RendererInterface *iface)
{
    return VTABLE_METHOD_CALL(iface, start);
}

static inline int32_t renderer_begin_frame(RendererInterface *iface, RendererCommandList **out_command_list)
{
    return VTABLE_METHOD_CALL(iface, begin_frame, out_command_list);
}

static inline int32_t renderer_end_frame(RendererInterface *iface)
{
    return VTABLE_METHOD_CALL(iface, end_frame);
}

static inline int32_t renderer_immediate_execute(RendererInterface *iface, ImmediateExecute_Fn immediate_execute_fn, void *user_data)
{
    return VTABLE_METHOD_CALL(iface, immediate_execute, immediate_execute_fn, user_data);
}

static inline RendererImageHandle renderer_get_render_image_handle(RendererInterface *iface)
{
    return VTABLE_METHOD_CALL_NO_ARGS(iface, get_render_image_handle);
}

static inline int32_t renderer_get_image_properties(RendererInterface *iface, RendererImageHandle image_handle, RendererImageProperties *out_image_properties)
{
    return VTABLE_METHOD_CALL(iface, get_image_properties, image_handle, out_image_properties);
}

static inline void renderer_on_window_resize(RendererInterface *iface, uint32_t width, uint32_t height)
{
    VTABLE_METHOD_CALL(iface, on_window_resize, width, height);
}

static inline bool renderer_consume_has_resized(RendererInterface *iface)
{
    return VTABLE_METHOD_CALL_NO_ARGS(iface, consume_has_resized);
}

static inline int32_t renderer_create_shader(RendererInterface *iface, const uint32_t *shader_code_u32, size_t shader_code_bytes_len, RendererShaderHandle *out_shader_handle)
{
    return VTABLE_METHOD_CALL(iface, create_shader, shader_code_u32, shader_code_bytes_len, out_shader_handle);
}

static inline int32_t renderer_destroy_shader(RendererInterface *iface, RendererShaderHandle shader_handle)
{
    return VTABLE_METHOD_CALL(iface, destroy_shader, shader_handle);
}

static inline int32_t renderer_create_image(RendererInterface *iface, RendererImageCreateInfo *renderer_image_create_info, RendererImageHandle *out_image_handle)
{
    return VTABLE_METHOD_CALL(iface, create_image, renderer_image_create_info, out_image_handle);
}

static inline int32_t renderer_destroy_image(RendererInterface *iface, RendererImageHandle image_handle)
{
    return VTABLE_METHOD_CALL(iface, destroy_image, image_handle);
}

static inline int32_t renderer_create_resource_set_layout(RendererInterface *iface, const RendererResourceSetLayoutCreateInfo *renderer_resource_set_layout_create_info, RendererResourceSetLayoutHandle *out_resource_set_layout_handle)
{
    return VTABLE_METHOD_CALL(iface, create_resource_set_layout, renderer_resource_set_layout_create_info, out_resource_set_layout_handle);
}

static inline int32_t renderer_destroy_resource_set_layout(RendererInterface *iface, RendererResourceSetLayoutHandle resource_set_layout_handle)
{
    return VTABLE_METHOD_CALL(iface, destroy_resource_set_layout, resource_set_layout_handle);
}

static inline int32_t renderer_allocate_transient_resource_set(RendererInterface *iface, RendererResourceSetLayoutHandle resource_set_layout_handle, RendererResourceSetHandle *out_resource_set_handle)
{
    return VTABLE_METHOD_CALL(iface, allocate_transient_resource_set, resource_set_layout_handle, out_resource_set_handle);
}

static inline void renderer_update_resource_set(RendererInterface *iface, const RendererResourceSetUpdateInfo *resource_set_update_info)
{
    VTABLE_METHOD_CALL(iface, update_resource_set, resource_set_update_info);
}

static inline int32_t renderer_create_pipeline_layout(RendererInterface *iface, const RendererPipelineLayoutCreateInfo *renderer_pipeline_layout_create_info, RendererPipelineLayoutHandle *out_pipeline_layout_handle)
{
    return VTABLE_METHOD_CALL(iface, create_pipeline_layout, renderer_pipeline_layout_create_info, out_pipeline_layout_handle);
}

static inline int32_t renderer_destroy_pipeline_layout(RendererInterface *iface, RendererPipelineLayoutHandle pipeline_layout_handle)
{
    return VTABLE_METHOD_CALL(iface, destroy_pipeline_layout, pipeline_layout_handle);
}

static inline int32_t renderer_create_graphics_pipeline(RendererInterface *iface, const RendererGraphicsPipelineCreateInfo *pipeline_create_info, RendererGraphicsPipelineHandle *out_pipeline_handle)
{
    return VTABLE_METHOD_CALL(iface, create_graphics_pipeline, pipeline_create_info, out_pipeline_handle);
}

static inline int32_t renderer_create_compute_pipeline(RendererInterface *iface, const RendererComputePipelineCreateInfo *pipeline_create_info, RendererComputePipelineHandle *out_pipeline_handle)
{
    return VTABLE_METHOD_CALL(iface, create_compute_pipeline, pipeline_create_info, out_pipeline_handle);
}

static inline int32_t renderer_destroy_graphics_pipeline(RendererInterface *iface, RendererGraphicsPipelineHandle pipeline_handle)
{
    return VTABLE_METHOD_CALL(iface, destroy_graphics_pipeline, pipeline_handle);
}

static inline int32_t renderer_destroy_compute_pipeline(RendererInterface *iface, RendererComputePipelineHandle pipeline_handle)
{
    return VTABLE_METHOD_CALL(iface, destroy_compute_pipeline, pipeline_handle);
}

static inline void renderer_cmd_begin_rendering(RendererInterface *iface, RendererCommandList *command_list, const RendererBeginRenderingInfo *renderer_begin_rendering_info)
{
    VTABLE_METHOD_CALL(iface, cmd_begin_rendering, command_list, renderer_begin_rendering_info);
}

static inline void renderer_cmd_end_rendering(RendererInterface *iface, RendererCommandList *command_list)
{
    VTABLE_METHOD_CALL(iface, cmd_end_rendering, command_list);
}

static inline void renderer_cmd_set_viewport(RendererInterface *iface, RendererCommandList *command_list, RendererExtent2D extent)
{
    VTABLE_METHOD_CALL(iface, cmd_set_viewport, command_list, extent);
}

static inline void renderer_cmd_set_scissor(RendererInterface *iface, RendererCommandList *command_list, RendererExtent2D extent)
{
    VTABLE_METHOD_CALL(iface, cmd_set_scissor, command_list, extent);
}

static inline void renderer_cmd_bind_graphics_pipeline(RendererInterface *iface, RendererCommandList *command_list, RendererGraphicsPipelineHandle pipeline_handle)
{
    VTABLE_METHOD_CALL(iface, cmd_bind_graphics_pipeline, command_list, pipeline_handle);
}

static inline void renderer_cmd_bind_compute_pipeline(RendererInterface *iface, RendererCommandList *command_list, RendererComputePipelineHandle pipeline_handle)
{
    VTABLE_METHOD_CALL(iface, cmd_bind_compute_pipeline, command_list, pipeline_handle);
}

static inline void renderer_cmd_bind_resource_sets(RendererInterface *iface, RendererCommandList *command_list, RendererPipelineType renderer_pipeline_type, RendererPipelineLayoutHandle pipeline_layout_handle, uint32_t first_set, uint32_t resource_set_len, const RendererResourceSetHandle *resource_set_handle, uint32_t dynamic_offset_len, const uint32_t *dynamic_offsets)
{
    VTABLE_METHOD_CALL(iface, cmd_bind_resource_sets, command_list, renderer_pipeline_type, pipeline_layout_handle, first_set, resource_set_len, resource_set_handle, dynamic_offset_len, dynamic_offsets);
}

static inline void renderer_cmd_push_constants(RendererInterface *iface, RendererCommandList *command_list, RendererPipelineLayoutHandle pipeline_layout_handle, RendererShaderStageFlags shader_stage_flags, uint32_t offset, uint32_t push_constants_size, void *push_constants)
{
    VTABLE_METHOD_CALL(iface, cmd_push_constants, command_list, pipeline_layout_handle, shader_stage_flags, offset, push_constants_size, push_constants);
}

static inline void renderer_cmd_dispatch(RendererInterface *iface, RendererCommandList *command_list, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)
{
    VTABLE_METHOD_CALL(iface, cmd_dispatch, command_list, group_count_x, group_count_y, group_count_z);
}

static inline void renderer_cmd_draw(RendererInterface *iface, RendererCommandList *command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
    VTABLE_METHOD_CALL(iface, cmd_draw, command_list, vertex_count, instance_count, first_vertex, first_instance);
}

static inline void renderer_cmd_transition_image(RendererInterface *iface, RendererCommandList *command_list, RendererImageHandle image_handle, RendererImageLayout renderer_current_layout, RendererImageLayout renderer_new_layout)
{
    VTABLE_METHOD_CALL(iface, cmd_transition_image, command_list, image_handle, renderer_current_layout, renderer_new_layout);
}

static inline void renderer_cmd_blit_image_to_image(RendererInterface *iface, RendererCommandList *command_list, RendererImageHandle image_handle_source, RendererImageHandle image_handle_destination, RendererExtent2D extent_source, RendererExtent2D extent_destination)
{
    VTABLE_METHOD_CALL(iface, cmd_blit_image_to_image, command_list, image_handle_source, image_handle_destination, extent_source, extent_destination);
}

#pragma pack(pop)
