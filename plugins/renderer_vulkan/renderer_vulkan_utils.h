#pragma once

#include <stdint.h>

#include <plugin_utils.h>

#define CREATE_VK_HANDLE_DEFINITION(object) \
    struct object##_T;                      \
    typedef struct object##_T *object;

#define VK_RETURN_IF_ERROR(logger, err_var, func_call, err_ret_val, ...) \
    RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, ((err_var) < VK_SUCCESS), func_call, err_ret_val, ##__VA_ARGS__)

#define VK_DESTROY_CHECKED(handle, destroy_func_call) \
    do                                                \
    {                                                 \
        if ((handle) != VK_NULL_HANDLE)               \
        {                                             \
            (destroy_func_call);                      \
            (handle) = VK_NULL_HANDLE;                \
        }                                             \
    } while (0)

#define VK_TRY_INIT(logger, err_var, create_func_call, cleanup_func_call, ...) \
    do                                                                                           \
    {                                                                                            \
        (err_var) = (create_func_call);                                                          \
        if ((err_var) < 0)                                                                       \
        {                                                                                        \
            LOG_ERR((logger), __VA_ARGS__);                                                      \
            (cleanup_func_call);                                                                 \
            return (err_var);                                                                    \
        }                                                                                        \
    } while (0)

struct LoggerInterface;

struct VkInstance_T;
typedef struct VkInstance_T *VkInstance;

typedef void (*vk_func_void_void)(void);

int32_t vk_get_instance_proc(struct LoggerInterface *logger, VkInstance instance, const char *proc_name, vk_func_void_void *out_func);