#pragma once
#ifndef PLUGIN_MANAGER_COMMON_H
#define PLUGIN_MANAGER_COMMON_H

#define STRINGIZE2(x) #x
#define STRINGIZE(x) STRINGIZE2(x)

#if defined(_MSC_VER)
#define TODO(msg) __pragma(message(__FILE__ "(" STRINGIZE(__LINE__) "): TODO: " msg))
#else // #if defined(_MSC_VER)
#define TODO(msg) _Pragma("message(\"TODO: " msg "\")")
#endif // #if defined(_MSC_VER)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

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

// TODO: Look at naming
#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)
#define UNIQUE_VAR(prefix) MACRO_CONCAT(prefix, __LINE__)

#define SAFE_WHILE(condition, max_iteration, on_fail) \
    for (uint32_t UNIQUE_VAR(_safety_loop_) = 0;      \
         (UNIQUE_VAR(_safety_loop_) <= (max_iters));  \
         UNIQUE_VAR(_safety_loop_)++)                 \
        if (UNIQUE_VAR(_safety_loop_) == (max_iters)) \
        {                                             \
            on_fail;                                  \
            break;                                    \
        }                                             \
        else if (!(condition))                        \
        {                                             \
            break;                                    \
        }                                             \
        else

#endif // PLUGIN_MANAGER_COMMON_H