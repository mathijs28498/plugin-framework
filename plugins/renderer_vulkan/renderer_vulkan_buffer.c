#include "renderer_vulkan_buffer.h"

#include <stdint.h>
#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_buffer, LOG_LEVEL_DEBUG)
#include <plugin_sdk/renderer/v1/renderer_interface.h>
#include <plugin_sdk/plugin_utils.h>

#include "renderer_vulkan_register.h"
#include "renderer_vulkan_utils.h"
#include "renderer_vulkan_conversion.h"
#include "renderer_vulkan.h"

int32_t renderer_vulkan_create_buffer(RendererContext *context, RendererBufferCreateInfo *renderer_buffer_create_info, RendererBufferHandle *out_buffer_handle)
{
    assert(context != NULL);
    assert(renderer_buffer_create_info != NULL);
    assert(out_buffer_handle != NULL);

    VkResult result;

    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = renderer_buffer_create_info->size,
        .usage = rv_buffer_usage_to_vk_buffer_usage(renderer_buffer_create_info->usage_flags),
    };

    TODO("Make the flags part of the buffer")
    VmaAllocationCreateInfo alloc_create_info = {
        .usage = rv_memory_usage_to_vma_memory_usage(renderer_buffer_create_info->memory_usage),
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
    };

    RV_AllocatedBuffer allocated_buffer;
    VmaAllocationInfo allocation_info;
    RV_RETURN_IF_ERROR(context->deps.logger, result, vmaCreateBuffer(context->vma_allocator, &buffer_create_info, &alloc_create_info, &allocated_buffer.buffer, &allocated_buffer.allocation, &allocation_info),
                       -1, "Failed to create buffer: %d", result);

    RendererVulkanHandle rv_buffer_handle = {0};
    RV_RES_RV_HANDLE_ALLOC_OR_RETURN(context->deps.logger, context->allocated_buffer_occupied_a, context->allocated_buffer_generations_a, context->allocated_buffers_a,
                                     allocated_buffer, rv_buffer_handle, vmaDestroyBuffer(context->vma_allocator, allocated_buffer.buffer, allocated_buffer.allocation));

    *out_buffer_handle = rv_buffer_handle.raw;
    return 0;
}