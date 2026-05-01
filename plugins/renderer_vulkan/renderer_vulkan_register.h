#pragma once

#include <cglm/types.h>

#include <stdbool.h>

#include <plugin_sdk/plugin_utils.h>

#include "plugin_dependencies.h"
#include "renderer_vulkan_utils.h"

#pragma pack(push, 8)

#define FRAMES_LEN 2
#define MAX_SWAPCHAIN_IMAGES_LEN 4

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
CREATE_VK_HANDLE_DEFINITION(VkDescriptorPool);
CREATE_VK_HANDLE_DEFINITION(VkDescriptorSetLayout);
CREATE_VK_HANDLE_DEFINITION(VkDescriptorSet);
CREATE_VK_HANDLE_DEFINITION(VkPipelineLayout);
CREATE_VK_HANDLE_DEFINITION(VkPipeline);
CREATE_VK_HANDLE_DEFINITION(VkBuffer);

CREATE_VK_HANDLE_DEFINITION(VmaAllocator);
CREATE_VK_HANDLE_DEFINITION(VmaAllocation);

typedef uint32_t RV_VkFormat;

typedef struct RV_VkExtent2D
{
    uint32_t width;
    uint32_t height;
} RV_VkExtent2D;

typedef struct RV_VkExtent3D
{
    uint32_t width;
    uint32_t height;
    uint32_t depth;
} RV_VkExtent3D;

typedef void (*rv_call_fn_any)(void);

typedef void (*rv_call_fn_1)(uint64_t);
typedef void (*rv_call_fn_2)(uint64_t, uint64_t);
typedef void (*rv_call_fn_3)(uint64_t, uint64_t, uint64_t);
typedef void (*rv_call_fn_4)(uint64_t, uint64_t, uint64_t, uint64_t);

typedef enum RV_CallType
{
    RV_CALL_TYPE_1,
    RV_CALL_TYPE_2,
    RV_CALL_TYPE_3,
    RV_CALL_TYPE_4,
} RV_CallType;

typedef struct RV_CallRecord
{
    RV_CallType call_type;
    uint64_t arg_0;
    uint64_t arg_1;
    uint64_t arg_2;
    uint64_t arg_3;
    rv_call_fn_any fn;
} RV_CallRecord;

typedef struct RendererFrameData
{
    VkCommandPool command_pool;
    VkCommandBuffer main_command_buffer;
    VkSemaphore swapchain_semaphore;
    VkSemaphore render_semaphore;
    VkFence render_fence;

    RV_CallRecord *destroy_queue;
} RendererFrameData;

typedef struct RV_AllocatedImage
{
    VkImage image;
    VkImageView image_view;
    VmaAllocation allocation;
    RV_VkExtent3D image_extent;
    RV_VkFormat image_format;
} RV_AllocatedImage;

typedef struct AllocatedBuffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
} AllocatedBuffer;

typedef struct Vertex
{

    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
} Vertex;

typedef uint64_t VkDeviceAddress;

typedef struct GPUMeshBuffers
{
    AllocatedBuffer index_buffer;
    AllocatedBuffer vertex_buffer;
    VkDeviceAddress vertex_buffer_address;
} GPUMeshBuffers;

typedef struct GPUDrawPushConstants
{
    mat4 world_matrix;
    VkDeviceAddress vertex_buffer_address;

} GPUDrawPushConstants;

TODO("Maybe split up the struct into smaller structs, like a queue/logical device struct")
TODO("The smaller struct could also be one for the bootstrap and one for runtime")
typedef struct RendererContext
{
    PluginDependencies deps;

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
    VkSwapchainKHR old_swapchain;
    bool resize_requested;
    bool halt_render;
    RV_VkExtent2D resize_extent;
    TODO("Figure out what to do with the size/capacity here")
    ARRAY_FIELD(VkImage, swapchain_images, MAX_SWAPCHAIN_IMAGES_LEN);
    ARRAY_FIELD(VkImageView, swapchain_image_views, MAX_SWAPCHAIN_IMAGES_LEN);
    RV_VkFormat swapchain_image_format;
    RV_VkExtent2D swapchain_extent;

    uint32_t frame_number;
    RendererFrameData frames[FRAMES_LEN];

    VmaAllocator vma_allocator;

    RV_AllocatedImage draw_image;
    RV_VkExtent2D draw_extent;

    RV_CallRecord *main_destroy_queue;
    RV_CallRecord *swapchain_destroy_queue;

    VkDescriptorPool global_descriptor_pool;

    VkDescriptorSetLayout draw_image_descriptor_set_layout;
    VkDescriptorSet draw_image_descriptor_set;

    VkPipelineLayout gradient_pipeline_layout;
    VkPipeline gradient_pipeline;
    VkPipeline triangle_pipeline;
    VkPipelineLayout mesh_pipeline_layout;
    VkPipeline mesh_pipeline;

    GPUMeshBuffers rectangle_mesh_buffers;

} RendererContext;

#pragma pack(pop)

typedef struct ComputePushConstants
{
    vec4 top_left;
    vec4 top_right;
    vec4 bottom_left;
    vec4 bottom_right;
} ComputePushConstants;
