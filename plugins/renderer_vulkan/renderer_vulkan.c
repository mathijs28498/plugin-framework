#include "renderer_vulkan.h"

#include <stdint.h>
#include <vulkan/vulkan.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(renderer_vulkan, LOG_LEVEL_DEBUG)

#include "renderer_vulkan_register.h"

int32_t renderer_vulkan_init(RendererContext *context)
{
    LoggerInterface *logger = context->logger;

    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkInstance instance;

    VkResult result = vkCreateInstance(&createInfo, NULL, &instance);
    (void)result;

    LOG_DBG("Created vk instance");

    return 0;
}