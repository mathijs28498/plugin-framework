#pragma once

#include <stdint.h>

#include <plugin_sdk/plugin_utils.h>

#define CREATE_VK_HANDLE_DEFINITION(object) \
    struct object##_T;                      \
    typedef struct object##_T *object;

#define VK_RETURN_IF_ERROR_CONDITION(logger, err_var, condition, func_call, err_ret_val, ...) \
    RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, (condition), func_call, err_ret_val, ##__VA_ARGS__)

#define VK_RETURN_IF_ERROR(logger, err_var, func_call, err_ret_val, ...) \
    RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, ((err_var) < VK_SUCCESS), func_call, err_ret_val, ##__VA_ARGS__)

    TODO("This does not flush multiple queues when for example the swapchain and the main queue have to be flushed, fix this")
#define VK_TRY_INIT(logger, err_var, create_func_call, destroy_queue, ...) \
    do                                                                     \
    {                                                                      \
        (err_var) = (create_func_call);                                    \
        if ((err_var) < 0)                                                 \
        {                                                                  \
            LOG_ERR((logger), __VA_ARGS__);                                \
            rv_call_queue_flush(destroy_queue);                      \
            return (err_var);                                              \
        }                                                                  \
    } while (0)

struct LoggerInterface;

struct VkInstance_T;
typedef struct VkInstance_T *VkInstance;

typedef void (*vk_func_void_void)(void);

int32_t vk_get_instance_proc(struct LoggerInterface *logger, VkInstance instance, const char *proc_name, vk_func_void_void *out_func);

struct RV_CallQueue;
struct RV_CallRecord;

typedef void (*rv_call_fn_any)(void);
int32_t rv_call_queue_push_1(struct LoggerInterface *logger, struct RV_CallQueue *queue, rv_call_fn_any fn, uint64_t arg_0);
int32_t rv_call_queue_push_2(struct LoggerInterface *logger, struct RV_CallQueue *queue, rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1);
int32_t rv_call_queue_push_3(struct LoggerInterface *logger, struct RV_CallQueue *queue, rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1, uint64_t arg_2);
int32_t rv_call_queue_push_4(struct LoggerInterface *logger, struct RV_CallQueue *queue, rv_call_fn_any fn, uint64_t arg_0, uint64_t arg_1, uint64_t arg_2, uint64_t arg_3);

#define RV_CALL_QUEUE_PUSH_1(logger, queue, fn, arg_0) \
    rv_call_queue_push_1((logger), (queue), (rv_call_fn_any)(fn), (uint64_t)(uintptr_t)(arg_0))
#define RV_CALL_QUEUE_PUSH_2(logger, queue, fn, arg_0, arg_1) \
    rv_call_queue_push_2((logger), (queue), (rv_call_fn_any)(fn), (uint64_t)(uintptr_t)(arg_0), (uint64_t)(uintptr_t)(arg_1))
#define RV_CALL_QUEUE_PUSH_3(logger, queue, fn, arg_0, arg_1, arg_2) \
    rv_call_queue_push_3((logger), (queue), (rv_call_fn_any)(fn), (uint64_t)(uintptr_t)(arg_0), (uint64_t)(uintptr_t)(arg_1), (uint64_t)(uintptr_t)(arg_2))
#define RV_CALL_QUEUE_PUSH_4(logger, queue, fn, arg_0, arg_1, arg_2, arg_3) \
    rv_call_queue_push_4((logger), (queue), (rv_call_fn_any)(fn), (uint64_t)(uintptr_t)(arg_0), (uint64_t)(uintptr_t)(arg_1), (uint64_t)(uintptr_t)(arg_2), (uint64_t)(uintptr_t)(arg_3))

void rv_call_queue_flush(struct RV_CallQueue *queue);

CREATE_VK_HANDLE_DEFINITION(VkSemaphore);
CREATE_VK_HANDLE_DEFINITION(VkCommandBuffer);
CREATE_VK_HANDLE_DEFINITION(VkImage);

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

typedef uint32_t VkImageAspectFlags; 
typedef uint64_t VkPipelineStageFlags2;

struct VkImageSubresourceRange rv_image_subresource_range(VkImageAspectFlags aspect_mask);
// ways to improve this efficiency: https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
void rv_transition_image(VkCommandBuffer cmd, VkImage image, enum VkImageLayout current_layout, enum VkImageLayout new_layout);
struct VkSemaphoreSubmitInfo rv_create_semaphore_submit_info(VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore);
struct VkCommandBufferSubmitInfo rv_create_command_buffer_submit_info(VkCommandBuffer cmd);
struct VkSubmitInfo2 rv_create_submit_info(struct VkCommandBufferSubmitInfo *cmd, struct VkSemaphoreSubmitInfo *signal_semaphore_info, struct VkSemaphoreSubmitInfo *wait_semaphore_info);

struct VkImageCreateInfo rv_create_image_info(enum VkFormat format, uint32_t usage_flags, struct VkExtent3D extent);
struct VkImageViewCreateInfo rv_create_image_view_info(enum VkFormat format, VkImage image, uint32_t aspect_mask);
void rv_copy_image_to_image(VkCommandBuffer cmd, VkImage source, VkImage destination, struct VkExtent2D src_size, struct VkExtent2D dst_size);