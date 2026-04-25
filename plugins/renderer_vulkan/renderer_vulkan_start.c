#include "renderer_vulkan_start.h"

#include <stdint.h>
#include <vulkan/vulkan.h>
#include <assert.h>

#include <vk_mem_alloc.h>

#include <plugin_sdk/plugin_utils.h>
#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_start, LOG_LEVEL_WARNING)

#include "shader_gradient.h"

#include "renderer_vulkan_bootstrap.h"
#include "renderer_vulkan_utils.h"
#include "renderer_vulkan_register.h"
#include "renderer_vulkan_descriptor_set.h"

int32_t create_command_buffers(RendererContext *context)
{
    assert(context != NULL);
    VkResult result;
    int32_t ret;

    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
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
        VK_RETURN_IF_ERROR(context->deps.logger, result,
                           vkCreateCommandPool(context->device, &command_pool_create_info, NULL, &frame->command_pool),
                           -1, "Failed to create command pool for frame %d: %d", i, result);

        RETURN_IF_ERROR(context->deps.logger, ret,
                        RV_CALL_QUEUE_PUSH_3(context->deps.logger, &context->main_destroy_queue, vkDestroyCommandPool, context->device, context->frames[i].command_pool, NULL),
                        "Failed to push command pool destroy data to destroy queue: %d", ret);

        command_buffer_alloc_info.commandPool = frame->command_pool;

        VK_RETURN_IF_ERROR(context->deps.logger, result,
                           vkAllocateCommandBuffers(context->device, &command_buffer_alloc_info, &frame->main_command_buffer),
                           -1, "Failed to allocate command buffer for frame %d: %d", i, result);
    }

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
        VK_RETURN_IF_ERROR(context->deps.logger, result, vkCreateFence(context->device, &fence_create_info, NULL, &frame->render_fence),
                           -1, "Failed to create fence for frame %d: %d", i, result);

        RETURN_IF_ERROR(context->deps.logger, ret,
                        RV_CALL_QUEUE_PUSH_3(context->deps.logger, &context->main_destroy_queue, vkDestroyFence, context->device, frame->render_fence, NULL),
                        "Failed to push render fence destroy data to destroy queue: %d", ret);

        VK_RETURN_IF_ERROR(context->deps.logger, result, vkCreateSemaphore(context->device, &semaphore_create_info, NULL, &frame->render_semaphore),
                           -1, "Failed to create render semaphore for frame %d: %d", i, result);

        RETURN_IF_ERROR(context->deps.logger, ret,
                        RV_CALL_QUEUE_PUSH_3(context->deps.logger, &context->main_destroy_queue, vkDestroySemaphore, context->device, frame->render_semaphore, NULL),
                        "Failed to push render semaphore destroy data to destroy queue: %d", ret);

        VK_RETURN_IF_ERROR(context->deps.logger, result, vkCreateSemaphore(context->device, &semaphore_create_info, NULL, &frame->swapchain_semaphore),
                           -1, "Failed to create swacphain semaphore for frame %d: %d", i, result);

        RETURN_IF_ERROR(context->deps.logger, ret,
                        RV_CALL_QUEUE_PUSH_3(context->deps.logger, &context->main_destroy_queue, vkDestroySemaphore, context->device, frame->swapchain_semaphore, NULL),
                        "Failed to push swapchain semaphore destroy data to destroy queue: %d", ret);
    }

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

    VK_RETURN_IF_ERROR(context->deps.logger, result, vmaCreateAllocator(&vma_alloc_create_info, &context->vma_allocator),
                       -1, "Unable to create vma allocator: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_1(context->deps.logger, &context->main_destroy_queue, vmaDestroyAllocator, context->vma_allocator),
                    "Failed to push swapchain semaphore destroy data to destroy queue: %d", ret);

    return 0;
}

int32_t create_draw_image(RendererContext *context)
{
    assert(context != NULL);

    VkResult result;
    uint32_t ret;

    VkExtent3D draw_image_extent = {
        .width = context->swapchain_extent.width,
        .height = context->swapchain_extent.height,
        .depth = 1,
    };

    VkFormat draw_image_format = VK_FORMAT_R16G16B16A16_SFLOAT;

    VkImageUsageFlags draw_image_usage_flags =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_STORAGE_BIT |
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo draw_image_create_info = rv_create_image_info(
        draw_image_format, draw_image_usage_flags, draw_image_extent);

    VmaAllocationCreateInfo allocation_create_info = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

    VK_RETURN_IF_ERROR(
        context->deps.logger, result,
        vmaCreateImage(context->vma_allocator, &draw_image_create_info, &allocation_create_info, &context->draw_image.image, &context->draw_image.allocation, NULL),
        -1, "Failed to create draw image: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, &context->swapchain_destroy_queue, vmaDestroyImage, context->vma_allocator, context->draw_image.image, context->draw_image.allocation),
                    "Failed to push image to destroy queue: %d", ret);

    VkImageViewCreateInfo draw_image_view_create_info = rv_create_image_view_info(draw_image_format, context->draw_image.image, VK_IMAGE_ASPECT_COLOR_BIT);

    VK_RETURN_IF_ERROR(
        context->deps.logger, result,
        vkCreateImageView(context->device, &draw_image_view_create_info, NULL, &context->draw_image.image_view),
        -1, "Failed to create draw image: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, &context->swapchain_destroy_queue, vkDestroyImageView, context->device, context->draw_image.image_view, NULL),
                    "Failed to push image view to destroy queue: %d", ret);

    context->draw_image.image_format = (RV_VkFormat)draw_image_format;
    context->draw_image.image_extent.width = draw_image_extent.width;
    context->draw_image.image_extent.height = draw_image_extent.height;
    context->draw_image.image_extent.depth = draw_image_extent.depth;

    return 0;
}

int32_t load_shader_module(RendererContext *context, const uint32_t *shader_code_u32, size_t shader_code_bytes_len, VkShaderModule *out_shader_module)
{
    assert(context != NULL);
    assert(out_shader_module != NULL);

    VkResult result;

    VkShaderModuleCreateInfo shader_module_create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pCode = shader_code_u32,
        .codeSize = shader_code_bytes_len,
    };

    VK_RETURN_IF_ERROR(
        context->deps.logger, result,
        vkCreateShaderModule(context->device, &shader_module_create_info, NULL, out_shader_module),
        -1, "Failed to create shader module: %d", result);

    return 0;
}

int32_t init_background_pipelines(RendererContext *context)
{
    assert(context != NULL);

    VkResult result;
    int32_t ret;

    VkPushConstantRange compute_push_constant_range = {
        .offset = 0,
        .size = sizeof(ComputePushConstants),
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    };

    VkPipelineLayoutCreateInfo compute_pipeline_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pSetLayouts = &context->draw_image_descriptor_set_layout,
        .setLayoutCount = 1,
        .pPushConstantRanges = &compute_push_constant_range,
        .pushConstantRangeCount = 1,
    };

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkCreatePipelineLayout(context->device, &compute_pipeline_layout_create_info, NULL, &context->gradient_pipeline_layout),
                       -1, "Failed to create gradient pipeline layout: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, &context->main_destroy_queue, vkDestroyPipelineLayout, context->device, context->gradient_pipeline_layout, NULL),
                    "Failed to push gradient pipeline layout destroy data to destroy queue: %d", ret);

    VkShaderModule compute_shader_module;
    RETURN_IF_ERROR(context->deps.logger, ret, load_shader_module(context, GRADIENT_SHADER_U32_CODE, GRADIENT_SHADER_BYTES_LEN, &compute_shader_module),
                    "Failed to load shader module: %d", ret);

    VkPipelineShaderStageCreateInfo stage_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = compute_shader_module,
        .pName = "main",
    };

    VkComputePipelineCreateInfo compute_pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .layout = context->gradient_pipeline_layout,
        .stage = stage_create_info,
    };

    VK_RETURN_IF_ERROR(context->deps.logger, result,
                       vkCreateComputePipelines(context->device, VK_NULL_HANDLE, 1, &compute_pipeline_create_info, NULL, &context->gradient_pipeline),
                       -1, "Failed to create compute pipelines: %d", result);

    RETURN_IF_ERROR(context->deps.logger, ret,
                    RV_CALL_QUEUE_PUSH_3(context->deps.logger, &context->main_destroy_queue, vkDestroyPipeline, context->device, context->gradient_pipeline, NULL),
                    "Failed to push gradient pipeline destroy data to destroy queue: %d", ret);

    TODO("Make sure it gets destroyed even if pipeline fails");
    vkDestroyShaderModule(context->device, compute_shader_module, NULL);

    return 0;
}

int32_t init_pipelines(RendererContext *context)
{
    assert(context != NULL);

    int32_t ret;

    RETURN_IF_ERROR(context->deps.logger, ret, init_background_pipelines(context),
                    "Failed to initialize background pipelines: %d", ret);
    assert(context != NULL);
    return 0;
}

int32_t renderer_vulkan_start(RendererContext *context)
{
    assert(context != NULL);

    int32_t ret;
    VK_TRY_INIT(context->deps.logger, ret, renderer_vulkan_bootstrap(context), &context->main_destroy_queue,
                "Failed to bootstrap vulkan: %d", ret);

    VK_TRY_INIT(context->deps.logger, ret, create_command_buffers(context), &context->main_destroy_queue,
                "Failed to init commands: %d", ret);

    VK_TRY_INIT(context->deps.logger, ret, create_sync_structures(context), &context->main_destroy_queue,
                "Failed to create sync structures: %d", ret);

    VK_TRY_INIT(context->deps.logger, ret, create_vma_allocator(context), &context->main_destroy_queue,
                "Failed to create vma allocator: %d", ret);

    VK_TRY_INIT(context->deps.logger, ret, create_draw_image(context), &context->main_destroy_queue,
                "Failed to create draw image: %d", ret);

    VK_TRY_INIT(context->deps.logger, ret, create_descriptor_sets(context), &context->main_destroy_queue,
                "Failed to create draw image: %d", ret);

    VK_TRY_INIT(context->deps.logger, ret, init_pipelines(context), &context->main_destroy_queue,
                "Failed to load shader module: %d", ret);

    return 0;
}

int32_t renderer_vulkan_start_recreate_swapchain(RendererContext *context)
{
    TODO("What needs to happen of fail? delete all queues?")
    assert(context != NULL);
    assert(context->device != VK_NULL_HANDLE);
    assert(context->swapchain != VK_NULL_HANDLE);

    int32_t ret;
    VkResult result;

    VK_RETURN_IF_ERROR(context->deps.logger, result, vkDeviceWaitIdle(context->device),
                       -1, "Failed to wait for device to idle: %d", result);

    rv_call_queue_flush(&context->swapchain_destroy_queue);

    if (context->resize_extent.width == 0 || context->resize_extent.height == 0)
    {
        context->resize_requested = false;
        context->halt_render = true;
        return 0;
    }

    context->halt_render = false;

    RETURN_IF_ERROR(context->deps.logger, ret, renderer_vulkan_bootstrap_recreate_swapchain(context),
                    "Failed to bootstrap recreate swapchain: %d", ret);

    RETURN_IF_ERROR(context->deps.logger, ret, create_draw_image(context),
                    "Failed to recreate draw image: %d", ret);

    RETURN_IF_ERROR(context->deps.logger, ret, create_descriptor_sets(context),
                    "Failed to recreate descriptor sets: %d", ret);

    context->resize_requested = false;
    return 0;
}