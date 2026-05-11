#include "renderer_vulkan_descriptor_set.h"

#include <stdint.h>
#include <vulkan/vulkan.h>
#include <plugin_sdk/plugin_utils.h>
#include <assert.h>

#include <bump_arena.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_descriptor_set, LOG_LEVEL_DEBUG)
#include <plugin_sdk/renderer/v1/renderer_interface.h>

#include "renderer_vulkan.h"
#include "renderer_vulkan_utils.h"
#include "renderer_vulkan_register.h"

int32_t create_descriptor_pool(RendererContext *context, uint32_t max_sets, VkDescriptorType *descriptor_types_a, VkDescriptorPool *out_descriptor_pool)
{
    assert(context != NULL);
    assert(max_sets > 0);
    assert(descriptor_types_a != NULL);

    VkResult result;
    int32_t ret;

    BumpArenaCheckpoint bump_arena_checkpoint = bump_arena_create_checkpoint(context->bump_arena_a);

    VkDescriptorPoolSize *descriptor_pool_sizes;
    RETURN_IF_ERROR(context->deps.logger, ret, BUMP_ARENA_ALLOC_TYPED(context->bump_arena_a, VkDescriptorPoolSize, GET_ARRAY_LENGTH(descriptor_types_a), &descriptor_pool_sizes),
                    "Failed to allocate from bump arena:%d", ret);

    ARRAY_FOR(descriptor_types_a, i)
    {
        descriptor_pool_sizes[i].type = descriptor_types_a[i];
        descriptor_pool_sizes[i].descriptorCount = max_sets;
    }

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = max_sets,
        .pPoolSizes = descriptor_pool_sizes,
        .poolSizeCount = (uint32_t)GET_ARRAY_LENGTH(descriptor_types_a),
    };

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkCreateDescriptorPool(context->device, &descriptor_pool_create_info, NULL, out_descriptor_pool),
                       -1, "Failed to create descriptor pool: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->main_destroy_queue, vkDestroyDescriptorPool, context->device, *out_descriptor_pool, NULL),
                    "Failed to push descriptor pool to swapchain destroy queue: %d", ret);

    bump_arena_restore_checkpoint(context->bump_arena_a, bump_arena_checkpoint, true);
    return 0;
}

int32_t renderer_vulkan_allocate_transient_resource_set(RendererContext *context, RendererResourceSetLayoutHandle resource_set_layout_handle, RendererResourceSetHandle *out_resource_set_handle)
{
    assert(context != NULL);
    assert(out_resource_set_handle != NULL);

    VkResult result;

    RendererFrameData *frame = context->active_frame_state.frame;
    size_t new_decriptor_set_index = GET_ARRAY_LENGTH(frame->transient_descriptor_sets);
    RETURN_IF_TRUE(context->deps.logger, new_decriptor_set_index >= GET_ARRAY_CAPACITY(frame->transient_descriptor_sets),
                   -1, "Failed to allocate transient descriptor set, transient descriptor sets for current frame is full");

    VkDescriptorSetLayout descriptor_set_layout;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->descriptor_set_layout_generations_a, context->descriptor_set_layouts_a, resource_set_layout_handle, descriptor_set_layout);

    VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = frame->transient_descriptor_pool,
        .pSetLayouts = &descriptor_set_layout,
        .descriptorSetCount = 1,
    };

    RV_RETURN_IF_ERROR(context->deps.logger, result,
                       vkAllocateDescriptorSets(context->device, &descriptor_set_alloc_info, &frame->transient_descriptor_sets[new_decriptor_set_index]),
                       -1, "Failed to allocate descriptor sets: %d", result);
    GET_ARRAY_LENGTH(frame->transient_descriptor_sets) += 1;
    *out_resource_set_handle = (uint64_t)new_decriptor_set_index;

    return 0;
}

int32_t renderer_vulkan_create_resource_set_layout(RendererContext *context, const RendererResourceSetLayoutCreateInfo *renderer_resource_set_layout_create_info, RendererResourceSetLayoutHandle *out_resource_set_layout_handle)
{
    assert(context != NULL);
    assert(renderer_create_resource_set_layout != NULL);

    VkResult result;
    int32_t ret;

    BumpArenaCheckpoint bump_arena_checkpoint = bump_arena_create_checkpoint(context->bump_arena_a);

    VkDescriptorSetLayoutBinding *descriptor_set_layout_bindings;
    uint32_t bindings_len = renderer_resource_set_layout_create_info->bindings_len;
    RETURN_IF_ERROR(context->deps.logger, ret, BUMP_ARENA_ALLOC_TYPED(context->bump_arena_a, VkDescriptorSetLayoutBinding, bindings_len, &descriptor_set_layout_bindings),
                    "Failed to allocate from bump arena: %d", ret);

    for (uint32_t i = 0; i < bindings_len; i++)
    {
        const RendererResourceSetLayoutBinding *renderer_binding = &renderer_resource_set_layout_create_info->bindings[i];
        VkDescriptorSetLayoutBinding *vk_binding = &descriptor_set_layout_bindings[i];
        vk_binding->binding = renderer_binding->binding;
        vk_binding->descriptorCount = renderer_binding->resource_len;
        vk_binding->descriptorType = rv_resource_type_to_vk_descriptor_type(renderer_binding->resource_type);
        vk_binding->stageFlags = rv_shader_stage_to_vk_shader_stage(renderer_binding->stage_flags);
        vk_binding->pImmutableSamplers = VK_NULL_HANDLE;
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = bindings_len,
        .pBindings = descriptor_set_layout_bindings,
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
    RV_RES_RV_HANDLE_ALLOC_OR_RETURN(context->deps.logger, context->descriptor_set_layout_occupied_a, context->descriptor_set_layout_generations_a, context->descriptor_set_layouts_a, descriptor_set_layout, rv_descriptor_set_layout_handle,
                                     vkDestroyDescriptorSetLayout(context->device, descriptor_set_layout, NULL));

    *out_resource_set_layout_handle = rv_descriptor_set_layout_handle.raw;

    bump_arena_restore_checkpoint(context->bump_arena_a, bump_arena_checkpoint, true);
    return 0;
}

int32_t rv_create_descriptor_pools(RendererContext *context)
{
    assert(context != NULL);

    int32_t ret;

    CREATE_INITIALIZED_ARRAY(VkDescriptorType, descriptor_pool_sizes,
                             {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE});

    RETURN_IF_ERROR(context->deps.logger, ret, create_descriptor_pool(context, 10, descriptor_pool_sizes, &context->global_descriptor_pool),
                    "Failed to create global descriptor pool: %d", ret);

    for (size_t i = 0; i < ARRAY_SIZE(context->frames); i++)
    {
        RETURN_IF_ERROR(context->deps.logger, ret, create_descriptor_pool(context, 10, descriptor_pool_sizes, &context->frames[i].transient_descriptor_pool),
                        "Failed to create transient descriptor pool: %d", ret);
    }

    return 0;
}

// TODO("Create a descriptor set handle for this, dont hardcode")
// int32_t renderer_vulkan_allocate_descriptor_set(RendererContext *context, RendererResourceSetLayoutHandle descriptor_set_layout_handle)
// {
//     assert(context != NULL);

//     int32_t ret;

//     VkDescriptorSetLayout descriptor_set_layout;
//     RendererVulkanHandle rv_descriptor_set_handle = {.raw = descriptor_set_layout_handle};
//     RV_RES_HANDLE_GET_OR_RETURN(context->deps.logger, context->descriptor_set_layout_generations_a, context->descriptor_set_layouts, rv_descriptor_set_handle, descriptor_set_layout);

//     RETURN_IF_ERROR(context->deps.logger, ret, allocate_descriptor_set(context, descriptor_set_layout, &context->draw_image_descriptor_set),
//                     "Failed to allocate descriptor set: %d", ret);

//     return 0;
// }

TODO("Add arguments for the infos")
TODO("Make generation decide if it is a transient or global")
void renderer_vulkan_update_transient_resource_set(RendererContext *context, RendererResourceSetHandle resource_set_handle)
{
    assert(context != NULL);

    RV_AllocatedImage allocated_image = {0};
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->allocated_image_generations_a, context->allocated_images_a,
                                              context->draw_image_handle, allocated_image);

    VkDescriptorImageInfo draw_image_descriptor_info = {
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        .imageView = allocated_image.image_view,
    };

    VkDescriptorSet descriptor_set = context->active_frame_state.frame->transient_descriptor_sets[(size_t)resource_set_handle];

    VkWriteDescriptorSet draw_image_write_descriptor_set = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = 0,
        .dstSet = descriptor_set,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &draw_image_descriptor_info,
    };

    vkUpdateDescriptorSets(context->device, 1, &draw_image_write_descriptor_set, 0, NULL);
}

// int32_t create_descriptor_sets(RendererContext *context)
// {
//     assert(context != NULL);

//     uint32_t ret;

//     RETURN_IF_ERROR(context->deps.logger, ret, create_descriptor_pool_and_set_layouts(context),
//                     "Failed to create descriptor pool and/or set layouts: %d", ret);

//     // RETURN_IF_ERROR(context->deps.logger, ret, renderer_vulkan_allocate_descriptor_set(context, context->draw_image_descriptor_set_layout_handle),
//     //                 "Failed to allocate descriptor set: %d", ret);

//     // renderer_vulkan_update_descriptor_set(context);

//     return 0;
// }