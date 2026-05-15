#include "renderer_vulkan_image.h"

#include <stdint.h>
#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include <plugin_sdk/plugin_utils.h>
#include <plugin_sdk/renderer/v1/renderer_interface.h>
#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_image, LOG_LEVEL_DEBUG)

#include "renderer_vulkan_register.h"
#include "renderer_vulkan_conversion.h"
#include "renderer_vulkan_utils.h"
#include "renderer_vulkan.h"

TODO("Add more options")
int32_t renderer_vulkan_create_image(RendererContext *context, RendererImageCreateInfo *renderer_image_create_info, RendererImageHandle *out_image_handle)
{
    assert(context != NULL);
    assert(renderer_image_create_info != NULL);
    assert(out_image_handle != NULL);

    VkResult result;
    int32_t ret;

    VkFormat vk_format = rv_image_format_to_vk_format(renderer_image_create_info->format);

    VkImageCreateInfo image_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = vk_format,
        .extent = EXTENT_3D_RENDERER_TO_VK(renderer_image_create_info->extent),
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = rv_image_usage_to_vk_image_usage(renderer_image_create_info->usage_flags),
    };

    VmaAllocationCreateInfo allocation_create_info = {
        .usage = rv_image_memory_usage_to_vma_memory_usage(renderer_image_create_info->memory_usage),
        .requiredFlags = rv_image_memory_usage_to_vk_memory_usage(renderer_image_create_info->memory_usage),
    };

    TODO("Get rid of RV_VkExtent3D ");
    RV_AllocatedImage allocated_image = {
        .image_format = renderer_image_create_info->format,
        .image_extent = renderer_image_create_info->extent,
    };

    RV_RETURN_IF_ERROR(context->deps.logger, result,
                       vmaCreateImage(context->vma_allocator, &image_create_info, &allocation_create_info, &allocated_image.image, &allocated_image.allocation, NULL),
                       -1, "Failed to create image: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->main_destroy_queue, vmaDestroyImage, context->vma_allocator, allocated_image.image, allocated_image.allocation),
                    "Failed to push image to destroy queue: %d", ret);

    VkImageSubresourceRange image_subresource_range = {
        .aspectMask = rv_vk_format_to_image_aspect(vk_format),
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    VkImageViewCreateInfo image_view_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .image = allocated_image.image,
        .format = vk_format,
        .subresourceRange = image_subresource_range,
    };

    RV_RETURN_IF_ERROR(context->deps.logger, result,
                       vkCreateImageView(context->device, &image_view_create_info, NULL, &allocated_image.image_view),
                       -1, "Failed to create image view: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->main_destroy_queue, vkDestroyImageView, context->device, allocated_image.image_view, NULL),
                    "Failed to push image view to destroy queue: %d", ret);

    RendererVulkanHandle rv_image_handle = {0};
    RV_RES_RV_HANDLE_ALLOC_OR_RETURN(context->deps.logger, context->allocated_image_occupied_a, context->allocated_image_generations_a, context->allocated_images_a,
                                     allocated_image, rv_image_handle,
                                     {
                                         vkDestroyImageView(context->device, allocated_image.image_view, NULL);
                                         vmaDestroyImage(context->vma_allocator, allocated_image.image, allocated_image.allocation);
                                     });

    *out_image_handle = rv_image_handle.raw;
    return 0;
}

RendererImageHandle renderer_vulkan_get_render_image_handle(RendererContext *context)
{
    assert(context != NULL);
    return context->swapchain_image_handles[context->active_frame_state.swapchain_index];
}

int32_t renderer_vulkan_get_image_properties(RendererContext *context, RendererImageHandle image_handle, RendererImageProperties *out_image_properties)
{
    assert(context != NULL);
    assert(out_image_properties != NULL);

    RV_AllocatedImage image = {0};
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->allocated_image_generations_a, context->allocated_images_a, image_handle, image);

    *out_image_properties = (RendererImageProperties){
        .extent = image.image_extent,
        .format = image.image_format,
    };

    return 0;
}
