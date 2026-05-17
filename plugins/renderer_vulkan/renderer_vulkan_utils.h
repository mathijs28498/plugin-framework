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
#define RV_TRY_INIT(logger, err_var, create_func_call, destroy_queue_a, ...) \
    do                                                                       \
    {                                                                        \
        (err_var) = (create_func_call);                                      \
        if ((err_var) < 0)                                                   \
        {                                                                    \
            LOG_ERR_TRACE((logger), __VA_ARGS__);                            \
            rv_call_queue_flush(destroy_queue_a);                            \
            return (err_var);                                                \
        }                                                                    \
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

RV_CREATE_HANDLE_DEFINITION(VmaAllocation);
RV_CREATE_HANDLE_DEFINITION(VmaAllocator);
RV_CREATE_HANDLE_DEFINITION(VkBuffer);

typedef uint32_t VkBufferUsageFlags;
typedef uint32_t VmaMemoryUsage;

struct AllocatedBuffer;
struct RendererContext;

TODO("Figure out if this belongs here, remove definitions if not")
int32_t rv_create_buffer(struct RendererContext *context, size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, struct AllocatedBuffer *out_buffer);
void rv_destroy_buffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation);

TODO("Figure out if O(1) free lookup is wanted")
#define RV_RES_RV_HANDLE_ALLOC(occupied_pool_a, generations_pool_a, resource_pool_a, resource, out_free_handle_found, out_handle) \
    do                                                                                                                            \
    {                                                                                                                             \
        (out_free_handle_found) = false;                                                                                          \
        ARRAY_FOR(resource_pool_a, UNIQUE_VAR(i))                                                                                 \
        {                                                                                                                         \
            if ((occupied_pool_a)[UNIQUE_VAR(i)])                                                                                 \
            {                                                                                                                     \
                continue;                                                                                                         \
            }                                                                                                                     \
            (out_free_handle_found) = true;                                                                                       \
            (occupied_pool_a)[UNIQUE_VAR(i)] = true;                                                                              \
            (resource_pool_a)[UNIQUE_VAR(i)] = (resource);                                                                        \
                                                                                                                                  \
            (out_handle).generation = (generations_pool_a)[UNIQUE_VAR(i)];                                                        \
            (out_handle).index = (uint32_t)UNIQUE_VAR(i);                                                                         \
            break;                                                                                                                \
        }                                                                                                                         \
    } while (0)

#define RV_RES_RV_HANDLE_ALLOC_OR_RETURN(logger, occupied_pool_a, generations_pool_a, resource_pool_a, resource, out_resource_handle, destroy_func) \
    do                                                                                                                                              \
    {                                                                                                                                               \
        bool UNIQUE_VAR(free_handle_found);                                                                                                         \
        RV_RES_RV_HANDLE_ALLOC(occupied_pool_a, generations_pool_a, resource_pool_a, resource, UNIQUE_VAR(free_handle_found), out_resource_handle); \
        if (!UNIQUE_VAR(free_handle_found))                                                                                                         \
        {                                                                                                                                           \
            LOG_ERR_TRACE(logger, "Failed to allocate handle, no free handle found");                                                               \
            destroy_func;                                                                                                                           \
            return -1;                                                                                                                              \
        }                                                                                                                                           \
    } while (0)

TODO("Maybe make the out_resource work by reference, should the input be a pointer or not then?")
#define RV_RES_RV_HANDLE_GET(generations_pool_a, resource_pool_a, rv_handle, out_ret, out_resource) \
    do                                                                                              \
    {                                                                                               \
        if ((rv_handle).index >= GET_ARRAY_CAPACITY(resource_pool_a))                               \
        {                                                                                           \
            (out_ret) = -1;                                                                         \
            break;                                                                                  \
        }                                                                                           \
        uint32_t UNIQUE_VAR(current_generation) = (generations_pool_a)[(rv_handle).index];          \
        if (UNIQUE_VAR(current_generation) != (rv_handle).generation)                               \
        {                                                                                           \
            (out_ret) = -2;                                                                         \
            break;                                                                                  \
        }                                                                                           \
        (out_resource) = (resource_pool_a)[(rv_handle).index];                                      \
        (out_ret) = 0;                                                                              \
    } while (0)

#define RV_RES_RENDERER_HANDLE_GET(generations_pool_a, resource_pool_a, handle, out_ret, out_resource)           \
    do                                                                                                           \
    {                                                                                                            \
        RendererVulkanHandle UNIQUE_VAR(rv_handle) = {.raw = (handle)};                                          \
        RV_RES_RV_HANDLE_GET(generations_pool_a, resource_pool_a, UNIQUE_VAR(rv_handle), out_ret, out_resource); \
    } while (0)

#define RV_RES_RV_HANDLE_GET_OR_RETURN(logger, generations_pool_a, resource_pool_a, rv_handle, out_resource) \
    do                                                                                                       \
    {                                                                                                        \
        int32_t UNIQUE_VAR(ret);                                                                             \
        RV_RES_RV_HANDLE_GET(generations_pool_a, resource_pool_a, rv_handle, UNIQUE_VAR(ret), out_resource); \
        if (UNIQUE_VAR(ret) < 0)                                                                             \
        {                                                                                                    \
            LOG_ERR_TRACE(logger, "Failed to get resource, invalid handle: %d", UNIQUE_VAR(ret));            \
            return UNIQUE_VAR(ret);                                                                          \
        }                                                                                                    \
    } while (0)

#define RV_RES_RENDERER_HANDLE_GET_OR_RETURN(logger, generations_pool_a, resource_pool_a, handle, out_resource)           \
    do                                                                                                                    \
    {                                                                                                                     \
        RendererVulkanHandle UNIQUE_VAR(rv_handle) = {.raw = (handle)};                                                   \
        RV_RES_RV_HANDLE_GET_OR_RETURN(logger, generations_pool_a, resource_pool_a, UNIQUE_VAR(rv_handle), out_resource); \
    } while (0)

#define RV_RES_RV_HANDLE_GET_OR_RETURN_VOID(logger, generations_pool_a, resource_pool_a, rv_handle, out_resource) \
    do                                                                                                            \
    {                                                                                                             \
        int32_t UNIQUE_VAR(ret);                                                                                  \
        RV_RES_RV_HANDLE_GET(generations_pool_a, resource_pool_a, rv_handle, UNIQUE_VAR(ret), out_resource);      \
        if (UNIQUE_VAR(ret) < 0)                                                                                  \
        {                                                                                                         \
            LOG_ERR_TRACE(logger, "Failed to get resource, invalid handle: %d", UNIQUE_VAR(ret));                 \
            return;                                                                                               \
        }                                                                                                         \
    } while (0)

#define RV_RES_RENDERER_HANDLE_GET_OR_RETURN_VOID(logger, generations_pool_a, resource_pool_a, handle, out_resource)           \
    do                                                                                                                         \
    {                                                                                                                          \
        RendererVulkanHandle UNIQUE_VAR(rv_handle) = {.raw = (handle)};                                                        \
        RV_RES_RV_HANDLE_GET_OR_RETURN_VOID(logger, generations_pool_a, resource_pool_a, UNIQUE_VAR(rv_handle), out_resource); \
    } while (0)

#define RV_RES_RV_HANDLE_FREE(occupied_pool_a, generations_pool_a, resource_pool_a, rv_handle, out_ret) \
    do                                                                                                  \
    {                                                                                                   \
        if ((rv_handle).index >= GET_ARRAY_CAPACITY(resource_pool_a))                                   \
        {                                                                                               \
            (out_ret) = -1;                                                                             \
            break;                                                                                      \
        }                                                                                               \
        uint32_t UNIQUE_VAR(is_occupied) = (occupied_pool_a)[(rv_handle).index];                        \
        if (!UNIQUE_VAR(is_occupied))                                                                   \
        {                                                                                               \
            (out_ret) = -2;                                                                             \
            break;                                                                                      \
        }                                                                                               \
        uint32_t UNIQUE_VAR(current_generation) = (generations_pool_a)[(rv_handle).index];              \
        if (UNIQUE_VAR(current_generation) != (rv_handle).generation)                                   \
        {                                                                                               \
            (out_ret) = -3;                                                                             \
            break;                                                                                      \
        }                                                                                               \
                                                                                                        \
        (occupied_pool_a)[(rv_handle).index] = false;                                                   \
        (generations_pool_a)[(rv_handle).index]++;                                                      \
        (out_ret) = 0;                                                                                  \
    } while (0)

#define RV_RES_RENDERER_HANDLE_FREE(occupied_pool_a, generations_pool_a, resource_pool_a, handle, out_ret)           \
    do                                                                                                               \
    {                                                                                                                \
        RendererVulkanHandle UNIQUE_VAR(rv_handle) = {.raw = (handle)};                                              \
        RV_RES_RV_HANDLE_FREE(occupied_pool_a, generations_pool_a, resource_pool_a, UNIQUE_VAR(rv_handle), out_ret); \
    } while (0)

#define RV_RES_RV_HANDLE_FREE_RETURN_IF_ERROR(logger, occupied_pool_a, generations_pool_a, resource_pool_a, rv_handle) \
    do                                                                                                                 \
    {                                                                                                                  \
        int32_t UNIQUE_VAR(ret);                                                                                       \
        RV_RES_RV_HANDLE_FREE(occupied_pool_a, generations_pool_a, resource_pool_a, rv_handle, UNIQUE_VAR(ret));       \
        if (UNIQUE_VAR(ret) < 0)                                                                                       \
        {                                                                                                              \
            LOG_ERR_TRACE(logger, "Failed to free resource, invalid handle: %d", UNIQUE_VAR(ret));                     \
            return UNIQUE_VAR(ret);                                                                                    \
        }                                                                                                              \
    } while (0)

#define RV_RES_RENDERER_HANDLE_FREE_RETURN_IF_ERROR(logger, occupied_pool_a, generations_pool_a, resource_pool_a, handle)           \
    do                                                                                                                              \
    {                                                                                                                               \
        RendererVulkanHandle UNIQUE_VAR(rv_handle) = {.raw = (handle)};                                                             \
        RV_RES_RV_HANDLE_FREE_RETURN_IF_ERROR(logger, occupied_pool_a, generations_pool_a, resource_pool_a, UNIQUE_VAR(rv_handle)); \
    } while (0)
