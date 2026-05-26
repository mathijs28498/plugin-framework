#include "renderer_vulkan_immediate.h"

#include <stdint.h>
#include <vulkan/vulkan.h>

#include <plugin_sdk/renderer/v1/renderer_interface.h>
#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_immediate, LOG_LEVEL_DEBUG)

#include "renderer_vulkan_register.h"
#include "renderer_vulkan_utils.h"

// 100 seconds
#define IMMEDIATE_FENCE_WAIT_TIMEOUT_NS 100ULL * 1000000000ULL

int32_t renderer_vulkan_immediate_execute(RendererContext *context, ImmediateExecute_Fn immediate_execute_fn, void *user_data)
{
    assert(context != NULL);
    assert(immediate_execute_fn != NULL);

    VkResult result;
    int32_t ret;

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkWaitForFences(context->device, 1, &context->immediate_fence, VK_TRUE, IMMEDIATE_FENCE_WAIT_TIMEOUT_NS),
                       -1, "Failed to wait for immediate fence: %d", result);
    TODO("Handle timeout reached");

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkResetFences(context->device, 1, &context->immediate_fence),
                       -1, "Failed to reset immediate fence: %d", result);

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkBeginCommandBuffer(context->immediate_command_buffer, &command_buffer_begin_info),
                       -1, "Failed to begin immediate command buffer: %d", -1);

    TODO("Figure out what to do when ret < 0");
    RendererCommandList command_list = {.command_buffer = context->immediate_command_buffer};
    ret = immediate_execute_fn(&command_list, user_data);

    RV_RETURN_IF_ERROR(context->deps.logger, result, vkEndCommandBuffer(context->immediate_command_buffer),
                       -1, "Failed to end immediate command buffer: %d", result);

    VkCommandBufferSubmitInfo buffer_submit_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = context->immediate_command_buffer,
    };

    VkSubmitInfo2 submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &buffer_submit_info,
    };
    RV_RETURN_IF_ERROR(context->deps.logger, result, vkQueueSubmit2(context->graphics_queue, 1, &submit_info, context->immediate_fence),
                       -1, "Failed to submit immediate command buffer to queue: %d", result);

    return ret;
}