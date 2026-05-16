#pragma once

#include <stddef.h>
#include <stdbool.h>

#include "plugin_utils_array.h"

#define STRINGIZE2(x) #x
#define STRINGIZE(x) STRINGIZE2(x)

#if defined(_MSC_VER)
// #define TODO(msg) __pragma(message(__FILE__ "(" STRINGIZE(__LINE__) "): TODO: " msg))
#define TODO(msg)
#else // #if defined(_MSC_VER)
#define TODO(msg) _Pragma("message(\"TODO: " msg "\")")
#endif // #if defined(_MSC_VER)

TODO("Add this somewhere else")
#define PLUGIN_CONTEXT_MEMORY_SLAB_SIZE 1024

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#if PLUGIN_BUILD_SHARED
static const bool IS_PLUGIN_BUILD_SHARED = true;
#else  // #if PLUGIN_BUILD_SHARED
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

#define CONCAT_2_(x, y) x##y
#define CONCAT_2(x, y) CONCAT_2_(x, y)
#define UNIQUE_VAR(prefix) CONCAT_2(prefix, __LINE__)

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

#define UNREACHABLE() \
    assert(0 && "Unreachable");

#define BITFIELD_SIZE_32(bits) (((bits) + 31) / 32)

#define CLAMP(value, low, high) ((value) < (low) ? (low) : ((high) < (value) ? (high) : (value)))

#define RETURN_IF_TRUE(logger, condition, err_ret_val, ...) \
    do                                                      \
    {                                                       \
        if ((condition))                                    \
        {                                                   \
            if ((logger) != NULL)                           \
                LOG_ERR_TRACE((logger), ##__VA_ARGS__);     \
            return (err_ret_val);                           \
        }                                                   \
    } while (0)

#define RETURN_IF_FALSE(logger, condition, err_ret_val, ...) \
    RETURN_IF_TRUE(logger, !(condition), err_ret_val, ##__VA_ARGS__)

#define RETURN_IF_TRUE_VOID(logger, condition, ...)     \
    do                                                  \
    {                                                   \
        if ((condition))                                \
        {                                               \
            if ((logger) != NULL)                       \
                LOG_ERR_TRACE((logger), ##__VA_ARGS__); \
            return;                                     \
        }                                               \
    } while (0)

#define RETURN_IF_FALSE_VOID(logger, condition, ...) \
    RETURN_IF_TRUE(logger, !(condition), ##__VA_ARGS__)

#define RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, condition, func_call, err_ret_val, ...) \
    do                                                                                               \
    {                                                                                                \
        (err_var) = (func_call);                                                                     \
        if ((condition))                                                                             \
        {                                                                                            \
            if ((logger) != NULL)                                                                    \
                LOG_ERR_TRACE((logger), ##__VA_ARGS__);                                              \
            return (err_ret_val);                                                                    \
        }                                                                                            \
    } while (0)

#define RETURN_IF_ERROR_CONDITION(logger, err_var, condition, func_call, ...) \
    RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, condition, func_call, err_var, ##__VA_ARGS__)

#define RETURN_IF_ERROR_RET_VALUE(logger, err_var, func_call, err_ret_val, ...) \
    RETURN_IF_ERROR_CONDITION_RET_VALUE(logger, err_var, ((err_var) < 0), func_call, err_ret_val, ##__VA_ARGS__)

#define RETURN_IF_ERROR(logger, err_var, func_call, ...) \
    RETURN_IF_ERROR_CONDITION(logger, err_var, ((err_var) < 0), func_call, ##__VA_ARGS__)

#define RETURN_IF_ERROR(logger, err_var, func_call, ...) \
    RETURN_IF_ERROR_CONDITION(logger, err_var, ((err_var) < 0), func_call, ##__VA_ARGS__)

#define RETURN_IF_ERROR_CONDITION_VOID(logger, err_var, condition, func_call, ...) \
    do                                                                             \
    {                                                                              \
        (err_var) = (func_call);                                                   \
        if ((condition))                                                           \
        {                                                                          \
            if ((logger) != NULL)                                                  \
                LOG_ERR_TRACE((logger), ##__VA_ARGS__);                            \
            return;                                                                \
        }                                                                          \
    } while (0)

#define RETURN_IF_ERROR_VOID(logger, err_var, func_call, ...) \
    RETURN_IF_ERROR_CONDITION_VOID(logger, err_var, ((err_var) < 0), func_call, ##__VA_ARGS__)
