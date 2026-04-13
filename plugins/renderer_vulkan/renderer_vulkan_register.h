#pragma once

#include <plugin_utils.h>

#include "plugin_dependencies.h"
#include "renderer_vulkan_utils.h"

#pragma pack(push, 8)

CREATE_VK_HANDLE_DEFINITION(VkDebugUtilsMessengerEXT);
CREATE_VK_HANDLE_DEFINITION(VkInstance);
CREATE_VK_HANDLE_DEFINITION(VkSurfaceKHR);
CREATE_VK_HANDLE_DEFINITION(VkPhysicalDevice);
CREATE_VK_HANDLE_DEFINITION(VkDevice);
CREATE_VK_HANDLE_DEFINITION(VkQueue);
CREATE_VK_HANDLE_DEFINITION(VkSwapchainKHR);
CREATE_VK_HANDLE_DEFINITION(VkImage);
CREATE_VK_HANDLE_DEFINITION(VkImageView);
CREATE_VK_HANDLE_DEFINITION(VkCommandPool);
CREATE_VK_HANDLE_DEFINITION(VkCommandBuffer);
CREATE_VK_HANDLE_DEFINITION(VkSemaphore);
CREATE_VK_HANDLE_DEFINITION(VkFence);

typedef uint32_t RV_VkFormat;

typedef struct RV_VkExtent2D
{
    uint32_t width;
    uint32_t height;
} RV_VkExtent2D;

typedef struct RendererFrameData
{
    VkCommandPool command_pool;
    VkCommandBuffer main_command_buffer;
    VkSemaphore swapchain_semaphore;
    VkSemaphore render_semaphore;
    VkFence render_fence;
} RendererFrameData;

#define FRAMES_LEN 2
#define MAX_SWAPCHAIN_IMAGES_LEN 4


TODO("Maybe split up the struct into smaller structs, like a queue/logical device struct")
TODO("The smaller struct could also be one for the bootstrap and one for runtime")
typedef struct RendererContext
{
    PLUGIN_CONTEXT_DEPENDENCIES

    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;

    VkDevice device;
    VkQueue graphics_queue;
    uint32_t graphics_queue_family;
    VkQueue present_queue;
    uint32_t present_queue_family;
    VkSwapchainKHR swapchain;
    TODO("Figure out what to do with the size/capacity here")
    ARRAY_FIELD(VkImage, swapchain_images, MAX_SWAPCHAIN_IMAGES_LEN);
    ARRAY_FIELD(VkImageView, swapchain_image_views, MAX_SWAPCHAIN_IMAGES_LEN);
    RV_VkFormat swapchain_image_format;
    RV_VkExtent2D swapchain_extent;

    uint32_t frame_number;
    RendererFrameData frames[FRAMES_LEN];
} RendererContext;

#pragma pack(pop)
