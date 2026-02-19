#pragma once

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

#if defined(_WIN32) && defined(_WINDOWS)
#define WINDOWS_GUI 1
#else // #if defined(_WIN32) && defined(_WINDOWS)
#define WINDOWS_GUI 0
#endif // #if defined(_WIN32) && defined(_WINDOWS)

// TODO: Look at naming
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

#ifndef STATIC_ASSERT
#define STATIC_ASSERT_CONCAT_(a, b) a##b
#define STATIC_ASSERT_CONCAT(a, b) STATIC_ASSERT_CONCAT_(a, b)

#define STATIC_ASSERT(expr, msg) \
    typedef char STATIC_ASSERT_CONCAT(assertion_failed_, __LINE__)[(expr) ? 1 : -1]
#endif

TODO("Make a macro for addressing any function that uses the context when you give the api")
#define NOT_IMPLEMENTED(return_type, ...) \
    ((void)(__VA_ARGS__),                 \
     assert(0 && "Not implemented"),      \
     (return_type)0)
    
#define BITFIELD_SIZE_32(bits) (((bits) + 31) / 32)