#pragma once

#include <stdint.h>

#include <plugin_sdk/renderer/v1/renderer_types.h>

struct RendererContext;
struct RendererBufferCreateInfo;
struct RendererCommandList;
struct RendererUploadBufferDataInfo;

int32_t renderer_vulkan_create_buffer(struct RendererContext *context, struct RendererBufferCreateInfo *renderer_buffer_create_info, RendererBufferHandle *out_buffer_handle);
int32_t renderer_vulkan_destroy_buffer(struct RendererContext *context, RendererBufferHandle buffer_handle);
int32_t renderer_vulkan_upload_buffer_data(struct RendererContext *context, struct RendererCommandList *command_list, struct RendererUploadBufferDataInfo *upload_buffer_data_info);
int32_t renderer_vulkan_get_buffer_device_address(struct RendererContext *context, RendererBufferHandle buffer_handle, RendererBufferDeviceAddress *out_device_address);