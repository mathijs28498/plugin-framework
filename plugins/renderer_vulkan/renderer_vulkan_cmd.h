#pragma once

#include <stdint.h>

#include <plugin_sdk/plugin_utils.h>
#include <plugin_sdk/renderer/v1/renderer_types.h>

TODO("Remoce this dependency")
#include <renderer_vulkan_utils.h>

TODO("Remove these")
RV_CREATE_HANDLE_DEFINITION(VkCommandBuffer);
enum VkImageLayout;

struct RendererContext;
struct RendererCommandList;

typedef uint32_t RendererShaderStageFlags;
enum RendererPipelineType;
struct RendererExtent2D;
enum RendererImageLayout;
struct RendererBeginRenderingInfo;

void renderer_vulkan_cmd_begin_render_pass(struct RendererContext *context, struct RendererCommandList *command_list);
void renderer_vulkan_cmd_bind_graphics_pipeline(struct RendererContext *context, struct RendererCommandList *command_list, RendererGraphicsPipelineHandle pipeline_handle);
void renderer_vulkan_cmd_bind_compute_pipeline(struct RendererContext *context, struct RendererCommandList *command_list, RendererComputePipelineHandle pipeline_handle);

void renderer_vulkan_cmd_bind_resource_sets(struct RendererContext *context, struct RendererCommandList *command_list, enum RendererPipelineType renderer_pipeline_type, RendererPipelineLayoutHandle pipeline_layout_handle, uint32_t first_set, uint32_t resource_set_len, const RendererResourceSetHandle *resource_set_handle, uint32_t dynamic_offset_len, const uint32_t *dynamic_offsets);
void renderer_vulkan_cmd_push_constants(struct RendererContext *context, struct RendererCommandList *command_list, RendererPipelineLayoutHandle pipeline_layout_handle, RendererShaderStageFlags shader_stage_flags, uint32_t offset, uint32_t push_constants_size, void *push_constants);
void renderer_vulkan_cmd_dispatch(struct RendererContext *context, struct RendererCommandList *command_list, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);

void renderer_vulkan_cmd_draw(struct RendererContext *context, struct RendererCommandList *command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);

TODO("Make platform agnostic")
// ways to improve this efficiency: https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
void renderer_vulkan_cmd_transition_image(struct RendererContext *context, struct RendererCommandList *command_list, RendererImageHandle image_handle, enum RendererImageLayout current_layout, enum RendererImageLayout new_layout);
void renderer_vulkan_cmd_blit_image_to_image(struct RendererContext *context, struct RendererCommandList *command_list, RendererImageHandle image_handle_source, RendererImageHandle image_handle_destination, struct RendererExtent2D extent_source, struct RendererExtent2D extent_destination);

void renderer_vulkan_cmd_begin_rendering(struct RendererContext *context, struct RendererCommandList *, const struct RendererBeginRenderingInfo *renderer_begin_rendering_info);
void renderer_vulkan_cmd_end_rendering(struct RendererContext *context, struct RendererCommandList *command_list);
void renderer_vulkan_cmd_set_viewport(struct RendererContext *context, struct RendererCommandList *command_list, struct RendererExtent2D extent);
void renderer_vulkan_cmd_set_scissor(struct RendererContext *context, struct RendererCommandList *command_list, struct RendererExtent2D extent);