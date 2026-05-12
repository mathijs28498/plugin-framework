#include "renderer_vulkan_utils.h"

#include <stdint.h>
#define VK_USE_64_BIT_PTR_DEFINES 1
#include <vulkan/vulkan.h>
#include <assert.h>

#include <vk_mem_alloc.h>

#include <plugin_sdk/plugin_utils.h>
#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_utils, LOG_LEVEL_DEBUG)
#include <plugin_sdk/renderer/v1/renderer_interface.h>

#include "renderer_vulkan_register.h"

int32_t vk_get_instance_proc(LoggerInterface *logger, VkInstance instance, const char *proc_name, vk_func_void_void *out_func)
{
    assert(logger != NULL);
    assert(instance != NULL);
    assert(proc_name != NULL);
    assert(out_func != NULL);

    RETURN_IF_ERROR_CONDITION_RET_VALUE(
        logger, *out_func, *out_func == NULL,
        (vk_func_void_void)vkGetInstanceProcAddr(instance, proc_name),
        (int32_t)VK_ERROR_EXTENSION_NOT_PRESENT,
        "Unable to find vk instance proc function '%s'", proc_name);

    return 0;
}

void rv_call_record_execute(RV_CallRecord *record)
{
    switch (record->call_type)
    {
    case RV_CALL_TYPE_1:
        ((rv_call_fn_1)(record->fn))(record->arg_0);
        break;
    case RV_CALL_TYPE_2:
        ((rv_call_fn_2)(record->fn))(record->arg_0, record->arg_1);
        break;
    case RV_CALL_TYPE_3:
        ((rv_call_fn_3)(record->fn))(record->arg_0, record->arg_1, record->arg_2);
        break;
    case RV_CALL_TYPE_4:
        ((rv_call_fn_4)(record->fn))(record->arg_0, record->arg_1, record->arg_2, record->arg_3);
        break;
    }
}

void rv_call_queue_flush(RV_CallRecord *call_queue)
{
    assert(call_queue != NULL);

    // Loop through the queue backwards as a LIFO queue
    for (size_t i = 0; i < GET_ARRAY_LENGTH(call_queue); i++)
    {
        size_t queue_index = GET_ARRAY_LENGTH(call_queue) - i - 1;
        RV_CallRecord *record = &call_queue[queue_index];

        rv_call_record_execute(record);
    }
    GET_ARRAY_LENGTH(call_queue) = 0;
}

TODO("Allow for a pointer to be registered to be set to 0 with memset and a size")
int32_t rv_call_queue_push_(LoggerInterface *logger, RV_CallRecord *call_queue, RV_CallType call_type,
                            rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1, uint64_t arg_2, uint64_t arg_3)
{
    assert(logger != NULL);
    assert(call_queue != NULL);
    assert(fn != NULL);

    size_t queue_len = GET_ARRAY_LENGTH(call_queue);
    if (queue_len >= GET_ARRAY_CAPACITY(call_queue))
    {
        LOG_ERR_TRACE(logger, "Failed to add destroy data to queue");

        RV_CallRecord record = {
            .call_type = call_type,
            .arg_0 = arg_0,
            .arg_1 = arg_1,
            .arg_2 = arg_2,
            .arg_3 = arg_3,
            .fn = fn,
        };
        rv_call_record_execute(&record);
        return -1;
    }

    call_queue[queue_len].call_type = call_type;
    call_queue[queue_len].arg_0 = arg_0;
    call_queue[queue_len].arg_1 = arg_1;
    call_queue[queue_len].arg_2 = arg_2;
    call_queue[queue_len].arg_3 = arg_3;
    call_queue[queue_len].fn = fn;

    GET_ARRAY_LENGTH(call_queue) += 1;

    return 0;
}

int32_t rv_call_queue_push_4(LoggerInterface *logger, RV_CallRecord *call_queue, rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1, uint64_t arg_2, uint64_t arg_3)
{
    return rv_call_queue_push_(logger, call_queue, RV_CALL_TYPE_4, fn, arg_0, arg_1, arg_2, arg_3);
}

int32_t rv_call_queue_push_3(LoggerInterface *logger, RV_CallRecord *call_queue, rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1, uint64_t arg_2)
{
    return rv_call_queue_push_(logger, call_queue, RV_CALL_TYPE_3, fn, arg_0, arg_1, arg_2, 0U);
}

int32_t rv_call_queue_push_2(LoggerInterface *logger, RV_CallRecord *call_queue, rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1)
{
    return rv_call_queue_push_(logger, call_queue, RV_CALL_TYPE_2, fn, arg_0, arg_1, 0U, 0U);
}

int32_t rv_call_queue_push_1(LoggerInterface *logger, RV_CallRecord *call_queue, rv_call_fn_any fn, uint64_t arg_0)
{
    return rv_call_queue_push_(logger, call_queue, RV_CALL_TYPE_1, fn, arg_0, 0U, 0U, 0U);
}

VkImageSubresourceRange rv_image_subresource_range(VkImageAspectFlags aspect_mask)
{
    return (VkImageSubresourceRange){
        .aspectMask = aspect_mask,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };
}

VkSemaphoreSubmitInfo rv_create_semaphore_submit_info(VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore)
{
    return (VkSemaphoreSubmitInfo){
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = semaphore,
        .stageMask = stage_mask,
        .deviceIndex = 0,
        .value = 1,
    };
}

VkCommandBufferSubmitInfo rv_create_command_buffer_submit_info(VkCommandBuffer cmd)
{
    return (VkCommandBufferSubmitInfo){
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
        .deviceMask = 0,
    };
}

VkSubmitInfo2 rv_create_submit_info(VkCommandBufferSubmitInfo *cmd, VkSemaphoreSubmitInfo *signal_semaphore_info, VkSemaphoreSubmitInfo *wait_semaphore_info)
{
    return (VkSubmitInfo2){
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,

        .waitSemaphoreInfoCount = wait_semaphore_info == NULL ? 0 : 1,
        .pWaitSemaphoreInfos = wait_semaphore_info,

        .signalSemaphoreInfoCount = signal_semaphore_info == NULL ? 0 : 1,
        .pSignalSemaphoreInfos = signal_semaphore_info,

        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = cmd,
    };
}

int32_t rv_create_buffer(RendererContext *context, size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, AllocatedBuffer *out_buffer)
{
    assert(alloc_size > 0);
    assert(out_buffer != NULL);

    VkResult result;

    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = alloc_size,
        .usage = usage,
    };

    VmaAllocationCreateInfo alloc_create_info = {
        .usage = memory_usage,
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
    };

    VmaAllocationInfo allocation_info;
    RV_RETURN_IF_ERROR(context->deps.logger, result, vmaCreateBuffer(context->vma_allocator, &buffer_create_info, &alloc_create_info, &out_buffer->buffer, &out_buffer->allocation, &allocation_info),
                       -1, "Failed to create buffer: %d", result);

    return 0;
}

void rv_destroy_buffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation)
{
    assert(allocator != VK_NULL_HANDLE);
    assert(buffer != NULL);
    vmaDestroyBuffer(allocator, buffer, allocation);
}

VkDescriptorType rv_resource_type_to_vk_descriptor_type(RendererResourceType resource_type)
{
    switch (resource_type)
    {
    case RENDERER_RESOURCE_TYPE_SAMPLER:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
        break;
    case RENDERER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        break;
    case RENDERER_RESOURCE_TYPE_SAMPLED_IMAGE:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        break;
    case RENDERER_RESOURCE_TYPE_STORAGE_IMAGE:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        break;

    default:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        break;
    }
}

VkShaderStageFlags rv_shader_stage_to_vk_shader_stage(RendererShaderStageFlags flags)
{
    VkShaderStageFlags vk_flags = 0;

    if (flags & RENDERER_SHADER_STAGE_VERTEX_BIT)
    {
        vk_flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (flags & RENDERER_SHADER_STAGE_FRAGMENT_BIT)
    {
        vk_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    if (flags & RENDERER_SHADER_STAGE_COMPUTE_BIT)
    {
        vk_flags |= VK_SHADER_STAGE_COMPUTE_BIT;
    }

    return vk_flags;
}

VkPipelineBindPoint rv_pipeline_type_to_vk_pipeline_bind_point(RendererPipelineType pipeline_type)
{
    switch (pipeline_type)
    {
    case RENDERER_PIPELINE_TYPE_GRAPHICS:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    case RENDERER_PIPELINE_TYPE_COMPUTE:
        return VK_PIPELINE_BIND_POINT_COMPUTE;
    default:
        return VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }
}

VkFormat rv_image_format_to_vk_format(RendererImageFormat format)
{
    switch (format)
    {
    case RENDERER_IMAGE_FORMAT_R8G8B8A8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case RENDERER_IMAGE_FORMAT_R8G8B8A8_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case RENDERER_IMAGE_FORMAT_R16G16B16A16_SFLOAT:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case RENDERER_IMAGE_FORMAT_R32G32B32A32_SFLOAT:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case RENDERER_IMAGE_FORMAT_D32_SFLOAT:
        return VK_FORMAT_D32_SFLOAT;
    case RENDERER_IMAGE_FORMAT_D24_UNORM_S8_UINT:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    default:
        return VK_FORMAT_UNDEFINED;
    }
}

VkImageUsageFlags rv_image_usage_to_vk_image_usage(RendererImageUsageFlags flags)
{
    VkImageUsageFlags vk_flags = 0;
    if (flags & RENDERER_IMAGE_USAGE_TRANSFER_SRC_BIT)
        vk_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (flags & RENDERER_IMAGE_USAGE_TRANSFER_DST_BIT)
        vk_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (flags & RENDERER_IMAGE_USAGE_SAMPLED_BIT)
        vk_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (flags & RENDERER_IMAGE_USAGE_STORAGE_BIT)
        vk_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (flags & RENDERER_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        vk_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (flags & RENDERER_IMAGE_USAGE_DEPTH_ATTACHMENT_BIT)
        vk_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    return vk_flags;
}

VkImageAspectFlags rv_vk_format_to_image_aspect(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D16_UNORM:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

VmaMemoryUsage rv_image_memory_usage_to_vma_memory_usage(RendererImageMemoryUsage memory_usage)
{
    switch (memory_usage)
    {
    case RENDERER_IMAGE_MEMORY_USAGE_GPU_ONLY:
        return VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    default:
        return VMA_MEMORY_USAGE_AUTO;
    }
}

VkMemoryPropertyFlags rv_image_memory_usage_to_vk_memory_usage(RendererImageMemoryUsage memory_usage)
{
    switch (memory_usage)
    {
    case RENDERER_IMAGE_MEMORY_USAGE_GPU_ONLY:
        return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    default:

        return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
}
