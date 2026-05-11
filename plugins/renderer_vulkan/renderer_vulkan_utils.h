#pragma once

#include <stdint.h>
#include <stddef.h>

#include <plugin_sdk/plugin_utils.h>

#define RV_CREATE_HANDLE_DEFINITION(object) \
    struct object##_T;                      \
    typedef struct object##_T *object;

#define RV_RETURN_IF_ERROR_CONDITION(logger, err_var, condition, func_call, err_ret_val, ...) \
    RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, (condition), func_call, err_ret_val, ##__VA_ARGS__)

#define RV_RETURN_IF_ERROR(logger, err_var, func_call, err_ret_val, ...) \
    RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, ((err_var) < VK_SUCCESS), func_call, err_ret_val, ##__VA_ARGS__)

TODO("This does not flush multiple queues when for example the swapchain and the main queue have to be flushed, fix this")
TODO("Is the flush even necessary with the new function? Can I just do it at the end in the start script where I destroy the init queue")
#define RV_TRY_INIT(logger, err_var, create_func_call, destroy_queue, ...) \
    do                                                                     \
    {                                                                      \
        (err_var) = (create_func_call);                                    \
        if ((err_var) < 0)                                                 \
        {                                                                  \
            LOG_ERR_TRACE((logger), __VA_ARGS__);                          \
            rv_call_queue_flush(destroy_queue);                            \
            return (err_var);                                              \
        }                                                                  \
    } while (0)

struct LoggerInterface;

struct VkInstance_T;
typedef struct VkInstance_T *VkInstance;

typedef void (*vk_func_void_void)(void);

int32_t vk_get_instance_proc(struct LoggerInterface *logger, VkInstance instance, const char *proc_name, vk_func_void_void *out_func);

struct RV_CallRecord;

typedef void (*rv_call_fn_any)(void);
int32_t rv_call_queue_push_1(struct LoggerInterface *logger, struct RV_CallRecord *call_queue, rv_call_fn_any fn, uint64_t arg_0);
int32_t rv_call_queue_push_2(struct LoggerInterface *logger, struct RV_CallRecord *call_queue, rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1);
int32_t rv_call_queue_push_3(struct LoggerInterface *logger, struct RV_CallRecord *call_queue, rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1, uint64_t arg_2);
int32_t rv_call_queue_push_4(struct LoggerInterface *logger, struct RV_CallRecord *call_queue, rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1, uint64_t arg_2, uint64_t arg_3);

#define RV_CALL_QUEUE_PUSH_1(logger, queue, fn, arg_0) \
    rv_call_queue_push_1((logger), (queue), (rv_call_fn_any)(fn), (uint64_t)(uintptr_t)(arg_0))
#define RV_CALL_QUEUE_PUSH_2(logger, queue, fn, arg_0, arg_1) \
    rv_call_queue_push_2((logger), (queue), (rv_call_fn_any)(fn), (uint64_t)(uintptr_t)(arg_0), (uint64_t)(uintptr_t)(arg_1))
#define RV_CALL_QUEUE_PUSH_3(logger, queue, fn, arg_0, arg_1, arg_2) \
    rv_call_queue_push_3((logger), (queue), (rv_call_fn_any)(fn), (uint64_t)(uintptr_t)(arg_0), (uint64_t)(uintptr_t)(arg_1), (uint64_t)(uintptr_t)(arg_2))
#define RV_CALL_QUEUE_PUSH_4(logger, queue, fn, arg_0, arg_1, arg_2, arg_3) \
    rv_call_queue_push_4((logger), (queue), (rv_call_fn_any)(fn), (uint64_t)(uintptr_t)(arg_0), (uint64_t)(uintptr_t)(arg_1), (uint64_t)(uintptr_t)(arg_2), (uint64_t)(uintptr_t)(arg_3))

void rv_call_queue_flush(struct RV_CallRecord *call_queue);

RV_CREATE_HANDLE_DEFINITION(VkSemaphore);
RV_CREATE_HANDLE_DEFINITION(VkCommandBuffer);
RV_CREATE_HANDLE_DEFINITION(VkImage);
RV_CREATE_HANDLE_DEFINITION(VkBuffer);

RV_CREATE_HANDLE_DEFINITION(VmaAllocator);
RV_CREATE_HANDLE_DEFINITION(VmaAllocation);

struct VkImageSubresourceRange;
enum VkImageLayout;
struct VkSemaphoreSubmitInfo;
struct VkSubmitInfo2;
struct VkCommandBufferSubmitInfo;
struct VkSemaphoreSubmitInfo;
struct VkExtent2D;
struct VkExtent3D;
struct VkImageViewCreateInfo;
enum VkFormat;
struct AllocatedBuffer;
struct RendererContext;

typedef uint32_t VkImageAspectFlags;
typedef uint64_t VkPipelineStageFlags2;
typedef uint32_t VkBufferUsageFlags;
typedef uint32_t VmaMemoryUsage;
struct VkExtent2D;
struct RV_VkExtent2D;

struct VkImageSubresourceRange rv_image_subresource_range(VkImageAspectFlags aspect_mask);
// ways to improve this efficiency: https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
void rv_transition_image(VkCommandBuffer cmd, VkImage image, enum VkImageLayout current_layout, enum VkImageLayout new_layout);
struct VkSemaphoreSubmitInfo rv_create_semaphore_submit_info(VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore);
struct VkCommandBufferSubmitInfo rv_create_command_buffer_submit_info(VkCommandBuffer cmd);
struct VkSubmitInfo2 rv_create_submit_info(struct VkCommandBufferSubmitInfo *cmd, struct VkSemaphoreSubmitInfo *signal_semaphore_info, struct VkSemaphoreSubmitInfo *wait_semaphore_info);

struct VkImageCreateInfo rv_create_image_info(enum VkFormat format, uint32_t usage_flags, struct VkExtent3D extent);
struct VkImageViewCreateInfo rv_create_image_view_info(enum VkFormat format, VkImage image, uint32_t aspect_mask);
void rv_copy_image_to_image(VkCommandBuffer cmd, VkImage source, VkImage destination, struct VkExtent2D src_size, struct VkExtent2D dst_size);
int32_t rv_create_buffer(struct RendererContext *context, size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, struct AllocatedBuffer *out_buffer);
void rv_destroy_buffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation);
struct VkExtent2D extent_2d(struct RV_VkExtent2D *rv_extent);

#define RV_RES_HANDLE_ALLOC(occupied_pool_a, generations_pool_a, resource_pool_a, resource, out_free_handle_found, out_handle) \
    do                                                                                                                         \
    {                                                                                                                          \
        (out_free_handle_found) = false;                                                                                       \
        ARRAY_FOR(resource_pool_a, UNIQUE_VAR(i))                                                                              \
        {                                                                                                                      \
            if ((occupied_pool_a)[UNIQUE_VAR(i)])                                                                              \
            {                                                                                                                  \
                continue;                                                                                                      \
            }                                                                                                                  \
            (out_free_handle_found) = true;                                                                                    \
            (occupied_pool_a)[UNIQUE_VAR(i)] = true;                                                                           \
            (resource_pool_a)[UNIQUE_VAR(i)] = (resource);                                                                     \
                                                                                                                               \
            (out_handle).generation = (generations_pool_a)[UNIQUE_VAR(i)];                                                     \
            (out_handle).index = (uint32_t)UNIQUE_VAR(i);                                                                      \
            break;                                                                                                             \
        }                                                                                                                      \
    } while (0)

#define RV_RES_HANDLE_ALLOC_OR_RETURN(logger, occupied_pool_a, generations_pool_a, resource_pool_a, resource, out_resource_handle, destroy_func) \
    do                                                                                                                                           \
    {                                                                                                                                            \
        bool UNIQUE_VAR(free_handle_found);                                                                                                      \
        RV_RES_HANDLE_ALLOC(occupied_pool_a, generations_pool_a, resource_pool_a, resource, UNIQUE_VAR(free_handle_found), out_resource_handle); \
        if (!UNIQUE_VAR(free_handle_found))                                                                                                      \
        {                                                                                                                                        \
            LOG_ERR_TRACE(logger, "Failed to allocate handle, no ");                                                                             \
            destroy_func;                                                                                                                        \
            return -1;                                                                                                                           \
        }                                                                                                                                        \
    } while (0)

#define RV_RES_HANDLE_GET(generations_pool_a, resource_pool_a, handle, out_ret, out_resource) \
    do                                                                                        \
    {                                                                                         \
        if ((handle).index >= GET_ARRAY_CAPACITY(resource_pool_a))                            \
        {                                                                                     \
            (out_resource) = VK_NULL_HANDLE;                                                  \
            (out_ret) = -1;                                                                   \
            break;                                                                            \
        }                                                                                     \
        uint32_t UNIQUE_VAR(current_generation) = (generations_pool_a)[(handle).index];       \
        if (UNIQUE_VAR(current_generation) != (handle).generation)                            \
        {                                                                                     \
            (out_resource) = VK_NULL_HANDLE;                                                  \
            (out_ret) = -2;                                                                   \
            break;                                                                            \
        }                                                                                     \
        (out_resource) = (resource_pool_a)[(handle).index];                                   \
        (out_ret) = 0;                                                                        \
    } while (0)

#define RV_RES_HANDLE_GET_OR_RETURN(logger, generations_pool_a, resource_pool_a, handle, out_resource) \
    do                                                                                                 \
    {                                                                                                  \
        int32_t UNIQUE_VAR(ret);                                                                       \
        RV_RES_HANDLE_GET(generations_pool_a, resource_pool_a, handle, UNIQUE_VAR(ret), out_resource); \
        if (UNIQUE_VAR(ret) < 0)                                                                       \
        {                                                                                              \
            LOG_ERR_TRACE(logger, "Failed to get resource, invalid handle: %d", UNIQUE_VAR(ret));      \
            return UNIQUE_VAR(ret);                                                                    \
        }                                                                                              \
    } while (0)

#define RV_RES_HANDLE_GET_OR_RETURN_VOID(logger, generations_pool_a, resource_pool_a, handle, out_resource) \
    do                                                                                                      \
    {                                                                                                       \
        int32_t UNIQUE_VAR(ret);                                                                            \
        RV_RES_HANDLE_GET(generations_pool_a, resource_pool_a, handle, UNIQUE_VAR(ret), out_resource);      \
        if (UNIQUE_VAR(ret) < 0)                                                                            \
        {                                                                                                   \
            LOG_ERR_TRACE(logger, "Failed to get resource, invalid handle: %d", UNIQUE_VAR(ret));           \
            return;                                                                                         \
        }                                                                                                   \
    } while (0)

#define RV_RES_HANDLE_FREE(occupied_pool_a, generations_pool_a, resource_pool_a, handle, out_ret) \
    do                                                                                            \
    {                                                                                             \
        if ((handle).index >= GET_ARRAY_CAPACITY(resource_pool_a))                                \
        {                                                                                         \
            (out_ret) = -1;                                                                       \
            break;                                                                                \
        }                                                                                         \
        uint32_t UNIQUE_VAR(current_generation) = (generations_pool_a)[(handle).index];           \
        if (UNIQUE_VAR(current_generation) != (handle).generation)                                \
        {                                                                                         \
            (out_ret) = -2;                                                                       \
            break;                                                                                \
        }                                                                                         \
                                                                                                  \
        (occupied_pool_a)[(handle).index] = false;                                                \
        (generations_pool_a)[(handle).index]++;                                                   \
        (out_ret) = 0;                                                                            \
    } while (0)

#define RV_RES_HANDLE_FREE_RETURN_IF_ERROR(logger, occupied_pool_a, generations_pool_a, resource_pool_a, handle) \
    do                                                                                                           \
    {                                                                                                            \
        int32_t UNIQUE_VAR(ret);                                                                                 \
        RV_RES_HANDLE_FREE(occupied_pool_a, generations_pool_a, resource_pool_a, handle, UNIQUE_VAR(ret));       \
        if (UNIQUE_VAR(ret) < 0)                                                                                 \
        {                                                                                                        \
            LOG_ERR_TRACE(logger, "Failed to free resource, invalid handle: %d", UNIQUE_VAR(ret));               \
            return UNIQUE_VAR(ret);                                                                              \
        }                                                                                                        \
    } while (0)

typedef uint32_t RendererShaderStageFlags;
typedef uint32_t VkShaderStageFlags;
typedef uint32_t VkImageUsageFlags;
typedef uint32_t RendererImageUsageFlags;
typedef uint32_t VmaMemoryUsage;
typedef uint32_t VkMemoryPropertyFlags;

enum RendererPipelineType;
enum VkPipelineBindPoint;
enum RendererResourceType;
enum VkDescriptorType;
enum RendererImageFormat;
enum VkImageUsageFlags;
enum VkFormat;
enum RendererImageMemoryUsage;

enum VkDescriptorType rv_resource_type_to_vk_descriptor_type(enum RendererResourceType);
VkShaderStageFlags rv_shader_stage_to_vk_shader_stage(RendererShaderStageFlags flags);
enum VkPipelineBindPoint rv_pipeline_type_to_vk_pipeline_bind_point(enum RendererPipelineType pipeline_type);
enum VkFormat rv_image_format_to_vk_format(enum RendererImageFormat format);
VkImageUsageFlags rv_image_usage_to_vk_image_usage(RendererImageUsageFlags flags);
VkImageAspectFlags rv_vk_format_to_image_aspect(enum VkFormat format);
VmaMemoryUsage rv_image_memory_usage_to_vma_memory_usage(enum RendererImageMemoryUsage memory_usage);
VkMemoryPropertyFlags rv_image_memory_usage_to_vk_memory_usage(enum RendererImageMemoryUsage memory_usage);

#define EXTENT_3D_RENDERER_TO_RV(renderer_extent) \
    (RV_VkExtent3D) { .width = (renderer_extent).width, .height = (renderer_extent).height, .depth = (renderer_extent).depth }

#define EXTENT_3D_RENDERER_TO_VK(renderer_extent) \
    (VkExtent3D) { .width = (renderer_extent).width, .height = (renderer_extent).height, .depth = (renderer_extent).depth }

#define EXTENT_3D_VK_TO_RENDERER(renderer_extent) \
    (RendererExtent3D) { .width = (vk_extent).width, .height = (vk_extent).height, .depth = (vk_extent).depth }

#define EXTENT_2D_RENDERER_TO_VK(renderer_extent) \
    (VkExtent2D) { .width = (renderer_extent).width, .height = (renderer_extent).height }

#define EXTENT_2D_VK_TO_RENDERER(vk_extent) \
    (RendererExtent3D) { .width = (vk_extent).width, .height = (vk_extent).height }
