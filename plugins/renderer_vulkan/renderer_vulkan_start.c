#include "renderer_vulkan_start.h"

#include <stdint.h>
#include <vulkan/vulkan.h>
#include <assert.h>
#include <string.h>

#include <vk_mem_alloc.h>
#include <bump_arena.h>

#include <plugin_sdk/plugin_utils.h>
#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_start, LOG_LEVEL_WARNING)
#include <plugin_sdk/renderer/v1/renderer_interface.h>

#include "renderer_vulkan_bootstrap.h"
#include "renderer_vulkan_utils.h"
#include "renderer_vulkan_register.h"
#include "renderer_vulkan_immediate.h"
#include "renderer_vulkan_descriptor_set.h"
#include "renderer_vulkan_pipeline.h"
#include "renderer_vulkan_image.h"
#include "renderer_vulkan_buffer.h"
#include "renderer_vulkan.h"

#define START_DESTROY_QUEUE_CAPACITY 64

int32_t create_frame_command_buffers(RendererContext *context)
{
    assert(context != NULL);

    VkResult result;
    int32_t ret;

    TODO("Test if the reset command buffer bit should be set")
    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = context->graphics_queue_family,
    };

    VkCommandBufferAllocateInfo command_buffer_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandBufferCount = 1,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };

    for (size_t i = 0; i < ARRAY_SIZE(context->frames); i++)
    {
        RendererFrameData *frame = &context->frames[i];
        RV_RETURN_IF_ERROR(context->deps.logger, result,
                           vkCreateCommandPool(context->device, &command_pool_create_info, NULL, &frame->command_pool),
                           -1, "Failed to create frame command pool for frame %d: %d", i, result);

        RETURN_IF_ERROR(context->deps.logger, ret,
                        RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->global_destroy_queue_a, vkDestroyCommandPool, context->device, context->frames[i].command_pool, NULL),
                        "Failed to push frame command pool destroy data to destroy queue: %d", ret);

        command_buffer_alloc_info.commandPool = frame->command_pool;

        RV_RETURN_IF_ERROR(context->deps.logger, result,
                           vkAllocateCommandBuffers(context->device, &command_buffer_alloc_info, &frame->command_list.command_buffer),
                           -1, "Failed to allocate frame command buffer for frame %d: %d", i, result);
    }
    return 0;
}

int32_t rv_immediate_start(RendererContext *context)
{
    assert(context != NULL);

    VkResult result;
    int32_t ret;

    TODO("Figure out if you should create a queue family for this");
    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = context->graphics_queue_family,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    };

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkCreateCommandPool(context->device, &command_pool_create_info, NULL, &context->immediate_command_pool),
                       -1, "Failed to create start command pool: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->active_frame_state.frame->destroy_queue_a, vkDestroyCommandPool, context->device, context->immediate_command_pool, NULL),
                    "Failed to push start command pool to destroy queue: %d", ret);

    VkCommandBufferAllocateInfo command_buffer_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandBufferCount = 1,
        .commandPool = context->immediate_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkAllocateCommandBuffers(context->device, &command_buffer_alloc_info, &context->immediate_command_buffer),
                       -1, "Failed to allocate start command buffer: %d", ret);

    return 0;
}

int32_t create_command_buffers(RendererContext *context)
{
    assert(context != NULL);

    int32_t ret;

    RETURN_IF_ERROR(context->deps.logger, ret, create_frame_command_buffers(context),
                    "Failed to create frame command buffers: %d", ret);

    RETURN_IF_ERROR(context->deps.logger, ret, rv_immediate_start(context),
                    "Failed to create init command buffers: %d", ret);

    return 0;
}

int32_t create_sync_structures(RendererContext *context)
{
    assert(context != NULL);

    VkResult result;
    int32_t ret;

    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    for (size_t i = 0; i < ARRAY_SIZE(context->frames); i++)
    {
        RendererFrameData *frame = &context->frames[i];
        RV_RETURN_IF_ERROR(context->deps.logger, result, vkCreateFence(context->device, &fence_create_info, NULL, &frame->render_fence),
                           -1, "Failed to create fence for frame %d: %d", i, result);

        RETURN_IF_ERROR(context->deps.logger, ret,
                        RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->global_destroy_queue_a, vkDestroyFence, context->device, frame->render_fence, NULL),
                        "Failed to push render fence destroy data to destroy queue: %d", ret);

        RV_RETURN_IF_ERROR(context->deps.logger, result, vkCreateSemaphore(context->device, &semaphore_create_info, NULL, &frame->render_semaphore),
                           -1, "Failed to create render semaphore for frame %d: %d", i, result);

        RETURN_IF_ERROR(context->deps.logger, ret,
                        RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->global_destroy_queue_a, vkDestroySemaphore, context->device, frame->render_semaphore, NULL),
                        "Failed to push render semaphore destroy data to destroy queue: %d", ret);

        RV_RETURN_IF_ERROR(context->deps.logger, result, vkCreateSemaphore(context->device, &semaphore_create_info, NULL, &frame->swapchain_semaphore),
                           -1, "Failed to create swacphain semaphore for frame %d: %d", i, result);

        RETURN_IF_ERROR(context->deps.logger, ret,
                        RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->global_destroy_queue_a, vkDestroySemaphore, context->device, frame->swapchain_semaphore, NULL),
                        "Failed to push swapchain semaphore destroy data to destroy queue: %d", ret);
    }

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkCreateFence(context->device, &fence_create_info, NULL, &context->immediate_fence),
                       -1, "Failed to immediate create fence: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, context->global_destroy_queue_a, vkDestroyFence, context->device, context->immediate_fence, NULL),
                    "Failed to push immediate fence destroy data to destroy queue: %d", ret);

    return 0;
}

int32_t create_vma_allocator(RendererContext *context)
{
    assert(context != NULL);

    VkResult result;
    int32_t ret;

    VmaAllocatorCreateInfo vma_alloc_create_info = {
        .physicalDevice = context->physical_device,
        .device = context->device,
        .instance = context->instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
    };

    RV_RETURN_IF_ERROR(context->deps.logger, result, vmaCreateAllocator(&vma_alloc_create_info, &context->vma_allocator),
                       -1, "Unable to create vma allocator: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_1(context->deps.logger, context->global_destroy_queue_a, vmaDestroyAllocator, context->vma_allocator),
                    "Failed to push swapchain semaphore destroy data to destroy queue: %d", ret);

    return 0;
}

int32_t renderer_vulkan_start_internal(RendererContext *context)
{
    assert(context != NULL);

    int32_t ret;

    RV_TRY_INIT(context->deps.logger, ret, renderer_vulkan_bootstrap(context), context->global_destroy_queue_a,
                "Failed to bootstrap vulkan: %d", ret);

    RV_TRY_INIT(context->deps.logger, ret, create_command_buffers(context), context->global_destroy_queue_a,
                "Failed to init commands: %d", ret);

    RV_TRY_INIT(context->deps.logger, ret, create_sync_structures(context), context->global_destroy_queue_a,
                "Failed to create sync structures: %d", ret);

    RV_TRY_INIT(context->deps.logger, ret, create_vma_allocator(context), context->global_destroy_queue_a,
                "Failed to create vma allocator: %d", ret);

    RV_TRY_INIT(context->deps.logger, ret, rv_create_descriptor_pools(context), context->global_destroy_queue_a,
                "Failed to create draw image: %d", ret);

    return 0;
}

int32_t renderer_vulkan_start(RendererContext *context)
{
    assert(context != NULL);
    int32_t ret;

    ret = renderer_vulkan_start_internal(context);

    bump_arena_free(context->bump_arena_a, true);

    return ret;
}

int32_t renderer_vulkan_start_recreate_swapchain(RendererContext *context)
{
    TODO("What needs to happen if fail? delete all queues?")
    assert(context != NULL);
    assert(context->device != VK_NULL_HANDLE);
    assert(context->swapchain != VK_NULL_HANDLE);

    VkResult result;
    int32_t ret;

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkDeviceWaitIdle(context->device),
                       -1, "Failed to wait for device to idle: %d", result);

    rv_call_queue_flush(context->swapchain_destroy_queue_a);

    if (context->resize_extent.width == 0 || context->resize_extent.height == 0)
    {
        context->resize_requested = false;
        context->halt_render = true;
        return 0;
    }

    context->halt_render = false;

    RETURN_IF_ERROR(context->deps.logger, ret, renderer_vulkan_bootstrap_recreate_swapchain(context),
                    "Failed to bootstrap recreate swapchain: %d", ret);

    context->resize_requested = false;
    context->swapchain_recreated = true;
    return 0;
}