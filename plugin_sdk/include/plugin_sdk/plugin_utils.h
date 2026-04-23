#pragma once

#include <stddef.h>
#include <stdbool.h>

#define PLUGIN_CONTEXT_MEMORY_SLAB_SIZE 1024

#define STRINGIZE2(x) #x
#define STRINGIZE(x) STRINGIZE2(x)

#if defined(_MSC_VER)
// #define TODO(msg) __pragma(message(__FILE__ "(" STRINGIZE(__LINE__) "): TODO: " msg))
#define TODO(msg)
#else // #if defined(_MSC_VER)
#define TODO(msg) _Pragma("message(\"TODO: " msg "\")")
#endif // #if defined(_MSC_VER)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#if PLUGIN_BUILD_SHARED
static const bool IS_PLUGIN_BUILD_SHARED = true;
#else // #if PLUGIN_BUILD_SHARED
static const bool IS_PLUGIN_BUILD_SHARED = false;
#endif // #if PLUGIN_BUILD_SHARED


#if defined(_DEBUG) || !defined(NDEBUG)
#define IS_DEBUG 1
#define IS_RELEASE 0
#else // #if defined(_DEBUG) || !defined(NDEBUG)
#define IS_DEBUG 0
#define IS_RELEASE 1
#endif // #if defined(_DEBUG) || !defined(NDEBUG)

#if IS_DEBUG
#define DEBUG_ONLY(code) code
#else // #if IS_DEBUG
#define DEBUG_ONLY(code)
#endif // #if IS_DEBUG

#ifndef WINDOWS_GUI
#define WINDOWS_GUI 0
#endif // #ifndef WINDOWS_GUI

#define SAFE_WHILE_CONCAT_(x, y) x##y
#define SAFE_WHILE_CONCAT(x, y) SAFE_WHILE_CONCAT_(x, y)
#define UNIQUE_VAR(prefix) SAFE_WHILE_CONCAT(prefix, __LINE__)

#define SAFE_WHILE(condition, max_iterations, on_fail)     \
    for (uint32_t UNIQUE_VAR(_safety_loop_) = 0;           \
         (UNIQUE_VAR(_safety_loop_) <= (max_iterations));  \
         UNIQUE_VAR(_safety_loop_)++)                      \
        if (UNIQUE_VAR(_safety_loop_) == (max_iterations)) \
        {                                                  \
            on_fail;                                       \
            break;                                         \
        }                                                  \
        else if (!(condition))                             \
        {                                                  \
            break;                                         \
        }                                                  \
        else

#define NOT_IMPLEMENTED(return_type, ...) \
    ((void)(__VA_ARGS__),                 \
     assert(0 && "Not implemented"),      \
     (return_type)0)

#define BITFIELD_SIZE_32(bits) (((bits) + 31) / 32)

#define CLAMP(value, low, high) ((value) < (low) ? (low) : ((high) < (value) ? (high) : (value)))

typedef union
{
    struct
    {
        size_t length;
        size_t capacity;
    };
    long long align_ll_;
    long double align_ld_;
    void *align_ptr_;
} ArrayHeader_;

#define ARRAY_FIELD(type, var_name, cap) \
    ArrayHeader_ header_##var_name##_;   \
    type var_name[cap]

#define INIT_ARRAY_FIELD(cap) {.header = {.length = 0, .capacity = (cap)}, .arr = {0}}

#define CREATE_ARRAY(type, var_name, cap) \
    struct                                \
    {                                     \
        ArrayHeader_ header;              \
        type arr[(cap)];                  \
    } var_name##_ = {                     \
        .header = {                       \
            .length = 0,                  \
            .capacity = (cap),            \
        },                                \
        .arr = {0}};                      \
    type *(var_name) = var_name##_.arr

#define CREATE_ARRAY_WITH_LEN(type, var_name, cap, len) \
    struct                                              \
    {                                                   \
        ArrayHeader_ header;                            \
        type arr[(cap)];                                \
    } var_name##_ = {                                   \
        .header = {                                     \
            .length = (len),                            \
            .capacity = (cap),                          \
        },                                              \
        .arr = {0}};                                    \
    type *(var_name) = var_name##_.arr

#define CREATE_ARRAY_FILLED(type, var_name, cap) \
    CREATE_ARRAY_WITH_LEN(type, var_name, cap, cap)

#define CREATE_ARRAY_WITH_DECL(decl, var_name, cap) \
    decl struct                                     \
    {                                               \
        ArrayHeader_ header;                        \
        type arr[cap];                              \
    } var_name##_ = {                               \
        .header = {                                 \
            .length = 0,                            \
            .capacity = cap,                        \
        },                                          \
        .arr = {0}};                                \
    decl type *(var_name) = var_name##_.arr

#define CREATE_INITIALIZED_ARRAY(type, var_name, ...)               \
    struct                                                          \
    {                                                               \
        ArrayHeader_ header;                                        \
        type arr[sizeof((type[])__VA_ARGS__) / sizeof(type)];       \
    } var_name##_ = {                                               \
        .header = {                                                 \
            .length = sizeof((type[])__VA_ARGS__) / sizeof(type),   \
            .capacity = sizeof((type[])__VA_ARGS__) / sizeof(type), \
        },                                                          \
        .arr = __VA_ARGS__};                                        \
    type *(var_name) = var_name##_.arr

#define CREATE_INITIALIZED_ARRAY_WITH_DECL(decl, type, var_name, ...) \
    decl struct                                                       \
    {                                                                 \
        ArrayHeader_ header;                                          \
        type arr[sizeof((type[])__VA_ARGS__) / sizeof(type)];         \
    } var_name##_ = {                                                 \
        .header = {                                                   \
            .length = sizeof((type[])__VA_ARGS__) / sizeof(type),     \
            .capacity = sizeof((type[])__VA_ARGS__) / sizeof(type),   \
        },                                                            \
        .arr = __VA_ARGS__};                                          \
    decl type *(var_name) = (type *)var_name##_.arr

#define GET_ARRAY_HEADER(arr_ptr) ((ArrayHeader_ *)(arr_ptr) - 1)
#define GET_ARRAY_CAPACITY(arr_ptr) (GET_ARRAY_HEADER(arr_ptr)->capacity)
#define GET_ARRAY_LENGTH(arr_ptr) (GET_ARRAY_HEADER(arr_ptr)->length)
#define SET_ARRAY_FIELD_CAPACITY(arr) GET_ARRAY_CAPACITY(arr) = ARRAY_SIZE(arr);

#define ARRAY_FOR(arr_ptr, index_name) \
    for (size_t index_name = 0; index_name < GET_ARRAY_LENGTH(arr_ptr); ++index_name)

#define ARRAY_PUSH_CHECKED(arr_ptr, element, on_err)                  \
    do                                                                \
    {                                                                 \
        if (GET_ARRAY_CAPACITY(arr_ptr) <= GET_ARRAY_LENGTH(arr_ptr)) \
        {                                                             \
            on_err                                                    \
        }                                                             \
        else                                                          \
        {                                                             \
            (arr_ptr)[GET_ARRAY_LENGTH(arr_ptr)] = (element);         \
            GET_ARRAY_LENGTH(arr_ptr) += 1;                           \
        }                                                             \
    } while (0)

#define ARRAY_PUSH_CHECKED_DEFAULT_RETURN(logger, arr_ptr, element)            \
    ARRAY_PUSH_CHECKED(arr_ptr, element, {                                     \
        LOG_ERR_TRACE(logger, "Unable to push to " #arr_ptr ", exceeding capacity"); \
        return -1;                                                             \
    })

#define ARRAY_PUSH_MULTI_CHECKED(arr_ptr, first_element, element_count, on_err)                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (GET_ARRAY_CAPACITY(arr_ptr) < GET_ARRAY_LENGTH(arr_ptr) + (element_count))                                 \
        {                                                                                                              \
            on_err                                                                                                     \
        }                                                                                                              \
        memcpy(&(arr_ptr)[GET_ARRAY_LENGTH(required_extensions)], (first_element), sizeof(arr_ptr) * (element_count)); \
        GET_ARRAY_LENGTH(arr_ptr) += (element_count);                                                                  \
    } while (0)

#define ARRAY_PUSH_ARRAY(arr_ptr, other_arr_ptr, on_err) \
    ARRAY_PUSH_MULTI_CHECKED(arr_ptr, other_arr_ptr, GET_ARRAY_LENGTH(other_arr_ptr), on_err)

#define RETURN_IF_TRUE(logger, condition, err_ret_val, ...) \
    do                                                      \
    {                                                       \
        if ((condition))                                    \
        {                                                   \
            if ((logger) != NULL)                           \
                LOG_ERR_TRACE((logger), ##__VA_ARGS__);           \
            return (err_ret_val);                           \
        }                                                   \
    } while (0)

#define RETURN_IF_FALSE(logger, condition, err_ret_val, ...) \
    RETURN_IF_TRUE(logger, !(condition), err_ret_val, ##__VA_ARGS__)

#define RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, condition, func_call, err_ret_val, ...) \
    do                                                                                               \
    {                                                                                                \
        (err_var) = (func_call);                                                                     \
        if ((condition))                                                                             \
        {                                                                                            \
            if ((logger) != NULL)                                                                    \
                LOG_ERR_TRACE((logger), ##__VA_ARGS__);                                                    \
            return (err_ret_val);                                                                    \
        }                                                                                            \
    } while (0)

#define RETURN_IF_ERROR_CONDITION(logger, err_var, condition, func_call, ...) \
    RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, condition, func_call, err_var, ##__VA_ARGS__)

#define RETURN_IF_ERROR_RET_VALUE(logger, err_var, func_call, err_ret_val, ...) \
    RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, ((err_var) < 0), func_call, err_ret_val, ##__VA_ARGS__)

#define RETURN_IF_ERROR(logger, err_var, func_call, ...) \
    RETURN_IF_ERROR_CONDITION(logger, err_var, ((err_var) < 0), func_call, ##__VA_ARGS__)
