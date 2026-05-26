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
#include "renderer_vulkan_conversion.h"
#include "renderer_vulkan.h"

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
