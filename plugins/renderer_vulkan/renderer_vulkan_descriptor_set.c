#include "renderer_vulkan_descriptor_set.h"

#include <stdint.h>
#include <vulkan/vulkan.h>
#include <plugin_sdk/plugin_utils.h>
#include <assert.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_descriptor_set, LOG_LEVEL_DEBUG)
#include <plugin_sdk/renderer/v1/renderer_interface.h>

#include "renderer_vulkan.h"
#include "renderer_vulkan_utils.h"
#include "renderer_vulkan_register.h"

#define MAX_DESCRIPTOR_POOL_SIZES_LEN 64

int32_t create_descriptor_pool(RendererContext *context, uint32_t max_sets, VkDescriptorType *descriptor_types, VkDescriptorPool *out_descriptor_pool)
{
    assert(context != NULL);
    assert(max_sets > 0);
    assert(descriptor_types != NULL);

    VkResult result;
    int32_t ret;

    TODO("Use an arena allocator here, calculate the amount needed/ just add more at the end with a reference to the start maybe?");
    CREATE_ARRAY(VkDescriptorPoolSize, descriptor_pool_sizes, MAX_DESCRIPTOR_POOL_SIZES_LEN);

    ARRAY_FOR(descriptor_types, i)
    {
        TODO("Check if the ratio from the tutorial for max_sets should be added");
        VkDescriptorPoolSize pool_size = {
            .type = descriptor_types[i],
            .descriptorCount = max_sets,
        };

        ARRAY_PUSH_CHECKED_DEFAULT_RETURN(context->deps.logger, descriptor_pool_sizes, pool_size);
    }

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = max_sets,
        .pPoolSizes = descriptor_pool_sizes,
        .poolSizeCount = (uint32_t)GET_ARRAY_LENGTH(descriptor_pool_sizes),
    };

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkCreateDescriptorPool(context->device, &descriptor_pool_create_info, NULL, out_descriptor_pool),
                       -1, "Failed to allocate descriptor pool: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->main_destroy_queue, vkDestroyDescriptorPool, context->device, context->global_descriptor_pool, NULL),
                    "Failed to push descriptor pool to swapchain destroy queue: %d", ret);

    return 0;
}

int32_t allocate_descriptor_set(RendererContext *context, VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet *out_descriptor_set)
{
    assert(context != NULL);
    assert(descriptor_set_layout != VK_NULL_HANDLE);
    assert(out_descriptor_set != NULL);

    VkResult result;

    VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = context->global_descriptor_pool,
        .pSetLayouts = &descriptor_set_layout,
        .descriptorSetCount = 1,
    };

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkAllocateDescriptorSets(context->device, &descriptor_set_alloc_info, out_descriptor_set),
                       -1, "Failed to allocate descriptor sets: %d", result);

    return 0;
}

TODO("Find proper name for this shite")
int32_t renderer_vulkan_create_descriptor_set(RendererContext *context, RendererDescriptorSetLayoutHandle *out_descriptor_set_layout_handle)
{
    assert(context != NULL);
    assert(out_descriptor_set_layout_handle != NULL);

    VkResult result;
    int32_t ret;

    TODO("Use arena and dynamic allocation here")
    TODO("Get data from arguments")
    CREATE_INITIALIZED_ARRAY(
        VkDescriptorSetLayoutBinding, descriptor_set_layout_bindings,
        {(VkDescriptorSetLayoutBinding){
            .binding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        }});

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pBindings = descriptor_set_layout_bindings,
        .bindingCount = (uint32_t)GET_ARRAY_LENGTH(descriptor_set_layout_bindings),
        .flags = 0,
    };

    VkDescriptorSetLayout descriptor_set_layout;
    RV_RETURN_IF_ERROR(context->deps.logger, result,
                       vkCreateDescriptorSetLayout(context->device, &descriptor_set_layout_create_info, NULL, &descriptor_set_layout),
                       -1, "Unable to create descriptor set layout: %d", result);

    TODO("This belongs in the main_destroy_queue, so shouldnt be here, but the whole function is being called in recreate_swapchain, so fix that")
    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->main_destroy_queue, vkDestroyDescriptorSetLayout, context->device, descriptor_set_layout, NULL),
                    "Failed to push descriptor set layout to swapchain destroy queue: %d", ret);

    RendererVulkanHandle rv_descriptor_set_layout_handle = {0};
    RV_RES_HANDLE_ALLOC_OR_RETURN(context->deps.logger, context->descriptor_set_layouts, context->descriptor_set_layout_generations, descriptor_set_layout, rv_descriptor_set_layout_handle,
                                  vkDestroyDescriptorSetLayout(context->device, descriptor_set_layout, NULL));

    *out_descriptor_set_layout_handle = rv_descriptor_set_layout_handle.raw;

    return 0;
}

TODO("Create methods for both")
int32_t create_descriptor_pool_and_set_layouts(RendererContext *context)
{
    assert(context != NULL);

    int32_t ret;

    CREATE_INITIALIZED_ARRAY(VkDescriptorType, descriptor_pool_sizes,
                             {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE});

    RETURN_IF_ERROR(context->deps.logger, ret, create_descriptor_pool(context, 10, descriptor_pool_sizes, &context->global_descriptor_pool),
                    "Failed to create global descriptor pool: %d", ret);

    RETURN_IF_ERROR(context->deps.logger, ret, renderer_vulkan_create_descriptor_set(context, &context->draw_image_descriptor_set_layout_handle),
                    "Failed to create descriptor set: %d", ret);

    return 0;
}

TODO("Create a descriptor set handle for this, dont hardcode")
int32_t renderer_vulkan_allocate_descriptor_set(RendererContext *context, RendererDescriptorSetLayoutHandle descriptor_set_layout_handle)
{
    assert(context != NULL);

    int32_t ret;

    VkDescriptorSetLayout descriptor_set_layout;
    RendererVulkanHandle rv_descriptor_set_handle = {.raw = descriptor_set_layout_handle};
    RV_RES_HANDLE_GET_OR_RETURN(context->deps.logger, context->descriptor_set_layouts, context->descriptor_set_layout_generations, rv_descriptor_set_handle, descriptor_set_layout);

    RETURN_IF_ERROR(context->deps.logger, ret, allocate_descriptor_set(context, descriptor_set_layout, &context->draw_image_descriptor_set),
                    "Failed to allocate descriptor set: %d", ret);

    return 0;
}

TODO("Add arguments for the infos")
void renderer_vulkan_update_descriptor_set(RendererContext *context)
{
    assert(context != NULL);

    VkDescriptorImageInfo draw_image_descriptor_info = {
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        .imageView = context->draw_image.image_view,
    };

    VkWriteDescriptorSet draw_image_write_descriptor_set = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = 0,
        .dstSet = context->draw_image_descriptor_set,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &draw_image_descriptor_info,
    };

    vkUpdateDescriptorSets(context->device, 1, &draw_image_write_descriptor_set, 0, NULL);
}

int32_t create_descriptor_sets(RendererContext *context)
{
    assert(context != NULL);

    uint32_t ret;

    RETURN_IF_ERROR(context->deps.logger, ret, create_descriptor_pool_and_set_layouts(context),
                    "Failed to create descriptor pool and/or set layouts: %d", ret);

    RETURN_IF_ERROR(context->deps.logger, ret, renderer_vulkan_allocate_descriptor_set(context, context->draw_image_descriptor_set_layout_handle),
                    "Failed to allocate descriptor set: %d", ret);

    renderer_vulkan_update_descriptor_set(context);

    return 0;
}