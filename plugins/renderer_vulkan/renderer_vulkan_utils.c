#include "renderer_vulkan_utils.h"

#include <stdint.h>
#define VK_USE_64_BIT_PTR_DEFINES 1 
#include <vulkan/vulkan.h>
#include <assert.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan_utils, LOG_LEVEL_DEBUG)

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