#pragma once

#include <stdint.h>

struct RendererContext;
struct RendererCommandList;
typedef uint64_t RendererGraphicsPipelineHandle;
typedef uint64_t RendererComputePipelineHandle;
typedef uint64_t RendererPipelineLayoutHandle;

enum RendererShaderStage;
enum RendererPipelineType;

void renderer_vulkan_cmd_begin_render_pass(struct RendererContext *context, struct RendererCommandList *command_list);
void renderer_vulkan_cmd_end_render_pass(struct RendererContext *context, struct RendererCommandList *command_list);
void renderer_vulkan_cmd_bind_graphics_pipeline(struct RendererContext *context, struct RendererCommandList *command_list, RendererGraphicsPipelineHandle pipeline_handle);
void renderer_vulkan_cmd_bind_compute_pipeline(struct RendererContext *context, struct RendererCommandList *command_list, RendererComputePipelineHandle pipeline_handle);

void renderer_vulkan_cmd_bind_descriptor_sets(struct RendererContext *context, struct RendererCommandList *command_list, enum RendererPipelineType renderer_pipeline_type, RendererPipelineLayoutHandle pipeline_layout_handle);
void renderer_vulkan_cmd_push_constants(struct RendererContext *context, struct RendererCommandList *command_list, RendererPipelineLayoutHandle pipeline_layout_handle, enum RendererShaderStage renderer_shader_stage, uint32_t offset, uint32_t push_constants_size, void *push_constants);
void renderer_vulkan_cmd_dispatch(struct RendererContext *context, struct RendererCommandList *command_list, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);

void renderer_vulkan_cmd_draw(struct RendererContext *context, struct RendererCommandList *command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);