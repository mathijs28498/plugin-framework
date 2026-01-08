#include <stdio.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "renderer_vk.h"

// TODO: Create a container for these
VkInstance instance = {0};

VkResult create_instance();

int32_t vk_init()
{
    VkResult res;

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
    printf("%s() - extension count: %d\n", __func__, extension_count);

    if (res = create_instance(&instance))
    {
        printf("%s() - ERROR: Unable to create instance: %d\n", __func__, res);
    }

    printf("%s() - Successful vk_init\n", __func__);
    return 0;
}


int32_t vk_cleanup()
{
    vkDestroyInstance(instance, NULL);
}

VkResult create_instance(VkInstance *instance)
{
    VkResult res = VK_SUCCESS;

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    // TODO: Figure out how get the needed extensions, like done with glfw in the tutorial:
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Instance

    create_info.enabledLayerCount = 0;

    if (res = vkCreateInstance(&create_info, NULL, &instance))
    {
        return res;
    }
}