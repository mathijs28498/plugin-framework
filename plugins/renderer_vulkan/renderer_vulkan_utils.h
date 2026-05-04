#pragma once

#include <stdint.h>
#include <stddef.h>

#include <plugin_sdk/plugin_utils.h>

#define CREATE_VK_HANDLE_DEFINITION(object) \
    struct object##_T;                      \
    typedef struct object##_T *object;

#define VK_RETURN_IF_ERROR_CONDITION(logger, err_var, condition, func_call, err_ret_val, ...) \
    RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, (condition), func_call, err_ret_val, ##__VA_ARGS__)

#define VK_RETURN_IF_ERROR(logger, err_var, func_call, err_ret_val, ...) \
    RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, ((err_var) < VK_SUCCESS), func_call, err_ret_val, ##__VA_ARGS__)

TODO("This does not flush multiple queues when for example the swapchain and the main queue have to be flushed, fix this")
TODO("Is the flush even necessary with the new function? Can I just do it at the end in the start script where I destroy the init queue")
#define VK_TRY_INIT(logger, err_var, create_func_call, destroy_queue, ...) \
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

CREATE_VK_HANDLE_DEFINITION(VkSemaphore);
CREATE_VK_HANDLE_DEFINITION(VkCommandBuffer);
CREATE_VK_HANDLE_DEFINITION(VkImage);
CREATE_VK_HANDLE_DEFINITION(VkBuffer);

CREATE_VK_HANDLE_DEFINITION(VmaAllocator);
CREATE_VK_HANDLE_DEFINITION(VmaAllocation);

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

#define RENDERER_VULKAN_RES_HANDLE_ALLOC(resource_pool, generations_pool, resource, out_free_handle_found, out_handle) \
    do                                                                                                                 \
    {                                                                                                                  \
        (out_free_handle_found) = false;                                                                               \
        ARRAY_FOR(resource_pool, UNIQUE_VAR(i))                                                                        \
        {                                                                                                              \
            if ((resource_pool)[UNIQUE_VAR(i)] != VK_NULL_HANDLE)                                                      \
            {                                                                                                          \
                continue;                                                                                              \
            }                                                                                                          \
            (resource_pool)[UNIQUE_VAR(i)] = (resource);                                                               \
            (out_free_handle_found) = true;                                                                            \
            (out_handle).generation = (generations_pool)[UNIQUE_VAR(i)];                                               \
            (out_handle).index = (uint32_t)UNIQUE_VAR(i);                                                              \
            break;                                                                                                     \
        }                                                                                                              \
    } while (0)

#define RENDERER_VULKAN_RES_HANDLE_GET(resource_pool, generations_pool, handle, out_ret, out_resource) \
    do                                                                                                 \
    {                                                                                                  \
        if ((handle).index >= GET_ARRAY_CAPACITY(resource_pool))                                       \
        {                                                                                              \
            (out_resource) = VK_NULL_HANDLE;                                                           \
            (out_ret) = -1;                                                                            \
            break;                                                                                     \
        }                                                                                              \
        uint32_t UNIQUE_VAR(current_generation) = (generations_pool)[(handle).index];                   \
        if (UNIQUE_VAR(current_generation) != (handle).generation)                                     \
        {                                                                                              \
            (out_resource) = VK_NULL_HANDLE;                                                           \
            (out_ret) = -2;                                                                            \
            break;                                                                                     \
        }                                                                                              \
        (out_resource) = (resource_pool)[(handle).index];                                              \
        (out_ret) = 0;                                                                                 \
    } while (0)

#define RENDERER_VULKAN_RES_HANDLE_GET_RETURN_IF_ERROR(resource_pool, generations_pool, handle, out_resource)   \
    do                                                                                                          \
    {                                                                                                           \
        int32_t UNIQUE_VAR(ret);                                                                                \
        RENDERER_VULKAN_RES_HANDLE_GET(resource_pool, generations_pool, handle, UNIQUE_VAR(ret), out_resource); \
        if (UNIQUE_VAR(ret) < 0)                                                                                \
        {                                                                                                       \
            LOG_ERR_TRACE(context->deps.logger, "Failed to get resource, invalid handle: %d", UNIQUE_VAR(ret)); \
            return UNIQUE_VAR(ret);                                                                             \
        }                                                                                                       \
    } while (0)

#define RENDERER_VULKAN_RES_HANDLE_FREE(resource_pool, generations_pool, handle, out_ret) \
    do                                                                                    \
    {                                                                                     \
        if ((handle).index >= GET_ARRAY_CAPACITY(resource_pool))                          \
        {                                                                                 \
            (out_ret) = -1;                                                               \
            break;                                                                        \
        }                                                                                 \
        uint32_t UNIQUE_VAR(current_generation) = (generations_pool)[(handle).index];      \
        if (UNIQUE_VAR(current_generation) != (handle).generation)                        \
        {                                                                                 \
            (out_ret) = -2;                                                               \
            break;                                                                        \
        }                                                                                 \
                                                                                          \
        (resource_pool)[(handle).index] = VK_NULL_HANDLE;                                 \
        (generations_pool)[(handle).index]++;                                             \
        (out_ret) = 0;                                                                    \
    } while (0)

#define RENDERER_VULKAN_RES_HANDLE_FREE_RETURN_IF_ERROR(resource_pool, generations_pool, handle)                 \
    do                                                                                                           \
    {                                                                                                            \
        int32_t UNIQUE_VAR(ret);                                                                                 \
        RENDERER_VULKAN_RES_HANDLE_FREE(resource_pool, generations_pool, handle, UNIQUE_VAR(ret));               \
        if (UNIQUE_VAR(ret) < 0)                                                                                 \
        {                                                                                                        \
            LOG_ERR_TRACE(context->deps.logger, "Failed to free resource, invalid handle: %d", UNIQUE_VAR(ret)); \
            return UNIQUE_VAR(ret);                                                                              \
        }                                                                                                        \
    } while (0)