#include "renderer_vulkan_utils.h"

#include <stdint.h>
#define VK_USE_64_BIT_PTR_DEFINES 1
#include <vulkan/vulkan.h>
#include <assert.h>

#include <plugin_sdk/plugin_utils.h>
#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_utils, LOG_LEVEL_DEBUG)

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

int32_t rv_call_queue_push_(LoggerInterface *logger, RV_CallQueue *queue, RV_CallType call_type,
                            rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1, uint64_t arg_2, uint64_t arg_3)
{
    assert(logger != NULL);
    assert(queue != NULL);
    assert(fn != NULL);

    if (queue->queue_len >= GET_ARRAY_CAPACITY(queue->queue))
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

    queue->queue[queue->queue_len].call_type = call_type;
    queue->queue[queue->queue_len].arg_0 = arg_0;
    queue->queue[queue->queue_len].arg_1 = arg_1;
    queue->queue[queue->queue_len].arg_2 = arg_2;
    queue->queue[queue->queue_len].arg_3 = arg_3;
    queue->queue[queue->queue_len].fn = fn;

    queue->queue_len += 1;

    return 0;
}

int32_t rv_call_queue_push_4(LoggerInterface *logger, RV_CallQueue *queue, rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1, uint64_t arg_2, uint64_t arg_3)
{
    return rv_call_queue_push_(logger, queue, RV_CALL_TYPE_4, fn, arg_0, arg_1, arg_2, arg_3);
}

int32_t rv_call_queue_push_3(LoggerInterface *logger, RV_CallQueue *queue, rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1, uint64_t arg_2)
{
    return rv_call_queue_push_(logger, queue, RV_CALL_TYPE_3, fn, arg_0, arg_1, arg_2, 0U);
}

int32_t rv_call_queue_push_2(LoggerInterface *logger, RV_CallQueue *queue, rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1)
{
    return rv_call_queue_push_(logger, queue, RV_CALL_TYPE_2, fn, arg_0, arg_1, 0U, 0U);
}

int32_t rv_call_queue_push_1(LoggerInterface *logger, RV_CallQueue *queue, rv_call_fn_any fn, uint64_t arg_0)
{

    return rv_call_queue_push_(logger, queue, RV_CALL_TYPE_1, fn, arg_0, 0U, 0U, 0U);
}

void rv_call_queue_flush(RV_CallQueue *queue)
{
    assert(queue != NULL);

    // Loop through the queue backwards as a LIFO queue
    for (size_t i = 0; i < queue->queue_len; i++)
    {
        size_t queue_index = queue->queue_len - i - 1;
        RV_CallRecord *record = &queue->queue[queue_index];

        rv_call_record_execute(record);
    }
    queue->queue_len = 0;
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

// ways to improve this efficiency: https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
void rv_transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout)
{
    VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageMemoryBarrier2 image_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
        .oldLayout = current_layout,
        .newLayout = new_layout,

        .subresourceRange = rv_image_subresource_range(aspect_mask),
        .image = image,
    };

    VkDependencyInfo dependency_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier,
    };

    vkCmdPipelineBarrier2(cmd, &dependency_info);
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

VkImageCreateInfo rv_create_image_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent)
{
    return (VkImageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage_flags,
    };
}

VkImageViewCreateInfo rv_create_image_view_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_mask)
{
    VkImageSubresourceRange rv_image_subresource_range = {
        .aspectMask = aspect_mask,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    return (VkImageViewCreateInfo){
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .image = image,
        .format = format,
        .subresourceRange = rv_image_subresource_range,
    };
}

void rv_copy_image_to_image(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size)
{
    VkOffset3D src_offset_max = {
        .x = src_size.width,
        .y = src_size.height,
        .z = 1,
    };

    VkOffset3D dst_offset_max = {
        .x = dst_size.width,
        .y = dst_size.height,
        .z = 1,
    };

    VkImageSubresourceLayers subresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseArrayLayer = 0,
        .layerCount = 1,
        .mipLevel = 0,
    };

    VkImageBlit2 blit_region = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
        .srcOffsets = {
            {.x = 0, .y = 0, .z = 0},
            src_offset_max,
        },
        .dstOffsets = {
            {.x = 0, .y = 0, .z = 0},
            dst_offset_max,
        },
        .srcSubresource = subresource,
        .dstSubresource = subresource,
    };

    VkBlitImageInfo2 blit_image_info = {
        .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
        .dstImage = destination,
        .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcImage = source,
        .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .filter = VK_FILTER_LINEAR,
        .regionCount = 1,
        .pRegions = &blit_region,
    };

    vkCmdBlitImage2(cmd, &blit_image_info);
}