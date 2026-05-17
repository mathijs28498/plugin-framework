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
#include "renderer_vulkan_conversion.h"

static inline int32_t inl_create_descriptor_pool(RendererContext *context, uint32_t max_sets, VkDescriptorType *descriptor_types_a, VkDescriptorPool *out_descriptor_pool)
{
    VkResult result;
    int32_t ret;

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
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->global_destroy_queue_a, vkDestroyDescriptorPool, context->device, *out_descriptor_pool, NULL),
                    "Failed to push descriptor pool to swapchain destroy queue: %d", ret);
    return 0;
}

int32_t create_descriptor_pool(RendererContext *context, uint32_t max_sets, VkDescriptorType *descriptor_types_a, VkDescriptorPool *out_descriptor_pool)
{
    assert(context != NULL);
    assert(max_sets > 0);
    assert(descriptor_types_a != NULL);

    BumpArenaCheckpoint bump_arena_checkpoint = bump_arena_create_checkpoint(context->bump_arena_a);
    int32_t ret = inl_create_descriptor_pool(context, max_sets, descriptor_types_a, out_descriptor_pool);
    bump_arena_restore_checkpoint(context->bump_arena_a, bump_arena_checkpoint, true);

    return ret;
}

int32_t renderer_vulkan_allocate_transient_resource_set(RendererContext *context, RendererResourceSetLayoutHandle resource_set_layout_handle, RendererResourceSetHandle *out_resource_set_handle)
{
    assert(context != NULL);
    assert(out_resource_set_handle != NULL);

    VkResult result;

    RendererFrameData *frame = context->active_frame_state.frame;
    size_t new_decriptor_set_index = GET_ARRAY_LENGTH(frame->transient_descriptor_sets_a);
    RETURN_IF_TRUE(context->deps.logger, new_decriptor_set_index >= GET_ARRAY_CAPACITY(frame->transient_descriptor_sets_a),
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
                       vkAllocateDescriptorSets(context->device, &descriptor_set_alloc_info, &frame->transient_descriptor_sets_a[new_decriptor_set_index]),
                       -1, "Failed to allocate descriptor sets: %d", result);
    GET_ARRAY_LENGTH(frame->transient_descriptor_sets_a) += 1;
    *out_resource_set_handle = (uint64_t)new_decriptor_set_index;

    return 0;
}

static inline int32_t inl_renderer_vulkan_create_resource_set_layout(RendererContext *context, const RendererResourceSetLayoutCreateInfo *renderer_resource_set_layout_create_info, RendererResourceSetLayoutHandle *out_resource_set_layout_handle)
{
    VkResult result;
    int32_t ret;

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

    RendererVulkanHandle rv_descriptor_set_layout_handle = {0};
    RV_RES_RV_HANDLE_ALLOC_OR_RETURN(context->deps.logger, context->descriptor_set_layout_occupied_a, context->descriptor_set_layout_generations_a, context->descriptor_set_layouts_a, descriptor_set_layout, rv_descriptor_set_layout_handle,
                                     vkDestroyDescriptorSetLayout(context->device, descriptor_set_layout, NULL));

    *out_resource_set_layout_handle = rv_descriptor_set_layout_handle.raw;
    return 0;
}

int32_t renderer_vulkan_create_resource_set_layout(RendererContext *context, const RendererResourceSetLayoutCreateInfo *renderer_resource_set_layout_create_info, RendererResourceSetLayoutHandle *out_resource_set_layout_handle)
{
    assert(context != NULL);
    assert(renderer_create_resource_set_layout != NULL);

    BumpArenaCheckpoint bump_arena_checkpoint = bump_arena_create_checkpoint(context->bump_arena_a);
    int32_t ret = inl_renderer_vulkan_create_resource_set_layout(context, renderer_resource_set_layout_create_info, out_resource_set_layout_handle);
    bump_arena_restore_checkpoint(context->bump_arena_a, bump_arena_checkpoint, true);

    return ret;
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

// This function allocates descriptor_image_infos on the bump_arena, make sure the arena is valid until the update descriptor set is called
void rv_create_descriptor_image_infos_from_resource_image_bindings(RendererContext *context, uint32_t resource_bindings_len, const RendererResourceImageBinding *resource_image_bindings, VkDescriptorImageInfo **out_descriptor_image_infos)
{
    assert(context != NULL);
    assert(resource_image_bindings != NULL);
    int32_t ret;
    *out_descriptor_image_infos = NULL;

    VkDescriptorImageInfo *descriptor_image_infos;
    RETURN_IF_ERROR_VOID(context->deps.logger, ret, BUMP_ARENA_ALLOC_TYPED(context->bump_arena_a, VkDescriptorImageInfo, resource_bindings_len, &descriptor_image_infos),
                         "Failed to allocate from bump arena: %d", ret);

    for (uint32_t i = 0; i < resource_bindings_len; i++)
    {
        const RendererResourceImageBinding *resource_image_binding = &resource_image_bindings[i];

        RV_AllocatedImage allocated_image = {0};
        RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(context->deps.logger, context->allocated_image_generations_a, context->allocated_images_a,
                                                  resource_image_binding->image_handle, allocated_image);

        descriptor_image_infos[i] = (VkDescriptorImageInfo){
            .imageLayout = rv_image_layout_to_vk_image_layout(resource_image_binding->image_layout),
            .imageView = allocated_image.image_view,
        };
    }

    *out_descriptor_image_infos = descriptor_image_infos;
}

VkWriteDescriptorSet rv_resource_set_update_write_to_vk_write_descriptor_set(RendererContext *context, VkDescriptorSet descriptor_set, const RendererResourceSetWrite *resource_set_update_write)
{
    assert(context != NULL);
    assert(descriptor_set != VK_NULL_HANDLE);
    assert(resource_set_update_write != NULL);

    VkWriteDescriptorSet write_descriptor_set = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor_set,
        .dstBinding = resource_set_update_write->binding,
        .dstArrayElement = resource_set_update_write->first_resource,
        .descriptorCount = resource_set_update_write->resource_bindings_len,
        .descriptorType = rv_resource_type_to_vk_descriptor_type(resource_set_update_write->resource_type),
    };

    switch (resource_set_update_write->resource_type)
    {
    case RENDERER_RESOURCE_TYPE_SAMPLED_IMAGE:
        TODO("To implement");
        UNREACHABLE();
        break;
    case RENDERER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER:
        TODO("To implement");
        UNREACHABLE();
        break;
    case RENDERER_RESOURCE_TYPE_SAMPLER:
        TODO("To implement");
        UNREACHABLE();
        break;
    case RENDERER_RESOURCE_TYPE_STORAGE_IMAGE:
        rv_create_descriptor_image_infos_from_resource_image_bindings(context, resource_set_update_write->resource_bindings_len, resource_set_update_write->image_bindings, &write_descriptor_set.pImageInfo);
        break;
    default:
        UNREACHABLE();
        break;
    }

    assert(write_descriptor_set.pImageInfo != NULL || write_descriptor_set.pBufferInfo != NULL);
    return write_descriptor_set;
}

void inl_renderer_vulkan_update_resource_set(RendererContext *context, const RendererResourceSetUpdateInfo *resource_set_update_info)
{
    int32_t ret;

    VkDescriptorSet descriptor_set = context->active_frame_state.frame->transient_descriptor_sets_a[(size_t)resource_set_update_info->resource_set_handle];

    VkWriteDescriptorSet *write_descriptor_sets;
    RETURN_IF_ERROR_VOID(context->deps.logger, ret, BUMP_ARENA_ALLOC_TYPED(context->bump_arena_a, VkWriteDescriptorSet, resource_set_update_info->resource_set_writes_len, &write_descriptor_sets),
                         "Failed to allocate from bump arena: %d", ret);

    for (uint32_t i = 0; i < resource_set_update_info->resource_set_writes_len; i++)
    {
        write_descriptor_sets[i] = rv_resource_set_update_write_to_vk_write_descriptor_set(context, descriptor_set, &resource_set_update_info->resource_set_writes[i]);
    }

    vkUpdateDescriptorSets(context->device, resource_set_update_info->resource_set_writes_len, write_descriptor_sets, 0, NULL);
}

TODO("Make generation decide if it is a transient or global")
void renderer_vulkan_update_resource_set(RendererContext *context, const RendererResourceSetUpdateInfo *resource_set_update_info)
{
    assert(context != NULL);
    assert(resource_set_update_info != NULL);

    BumpArenaCheckpoint bump_arena_checkpoint = bump_arena_create_checkpoint(context->bump_arena_a);
    inl_renderer_vulkan_update_resource_set(context, resource_set_update_info);
    bump_arena_restore_checkpoint(context->bump_arena_a, bump_arena_checkpoint, true);
}

int32_t renderer_vulkan_destroy_resource_set_layout(RendererContext *context, RendererResourceSetLayoutHandle resource_set_layout_handle)
{
    assert(context != NULL);

    int32_t ret;

    VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
    RV_RES_RENDERER_HANDLE_GET_OR_RETURN(context->deps.logger, context->descriptor_set_layout_generations_a, context->descriptor_set_layouts_a, resource_set_layout_handle, descriptor_set_layout);
    RV_RES_RENDERER_HANDLE_FREE_RETURN_IF_ERROR(context->deps.logger, context->descriptor_set_layout_occupied_a, context->descriptor_set_layout_generations_a, context->descriptor_set_layouts_a, resource_set_layout_handle);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->active_frame_state.frame->destroy_queue_a, vkDestroyDescriptorSetLayout, context->device, descriptor_set_layout, NULL),
                    "Failed to push descriptor set layout to swapchain destroy queue: %d", ret);

    return 0;
}