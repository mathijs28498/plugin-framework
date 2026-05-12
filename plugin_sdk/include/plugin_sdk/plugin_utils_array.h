#pragma once

#include <assert.h>
#include <stddef.h>

#ifndef MAX_ALIGN
#define MAX_ALIGN 16 // safe for x86/x86-64; use 8 for 32-bit ARM
#endif               // #ifndef MAX_ALIGN
_Static_assert(MAX_ALIGN >= _Alignof(double), "MAX_ALIGN too small for double");
_Static_assert(MAX_ALIGN >= _Alignof(void *), "MAX_ALIGN too small for pointer");

typedef struct
{
    _Alignas(MAX_ALIGN) size_t length;
    size_t capacity;
} ArrayHeader_;

#define ARRAY_FIELD(type, var_name, cap) \
    ArrayHeader_ header_##var_name##_;   \
    type var_name[cap]

#define INIT_ARRAY_FIELD(cap) {.header = {.length = 0, .capacity = (cap)}, .arr = {0}}

#define BIND_ARRAY_WITH_LEN(type, mem_ptr, arr_ptr, cap, len)        \
    do                                                               \
    {                                                                \
        ArrayHeader_ *internal_arr_hdr_ = (ArrayHeader_ *)(mem_ptr); \
        internal_arr_hdr_->length = (len);                           \
        internal_arr_hdr_->capacity = (cap);                         \
        (arr_ptr) = (type *)(internal_arr_hdr_ + 1);                 \
    } while (0)

#define BIND_ARRAY(type, mem_ptr, arr_ptr, cap) \
    BIND_ARRAY_WITH_LEN(type, mem_ptr, arr_ptr, cap, 0)

#define BIND_ARRAY_FILLED(type, mem_ptr, arr_ptr, cap) \
    BIND_ARRAY_WITH_LEN(type, mem_ptr, arr_ptr, cap, cap)

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

#define CREATE_ARRAY(type, var_name, cap) \
    CREATE_ARRAY_WITH_LEN(type, var_name, cap, 0)

#define CREATE_ARRAY_FILLED(type, var_name, cap) \
    CREATE_ARRAY_WITH_LEN(type, var_name, cap, cap)

#define CREATE_ARRAY_WITH_DECL(decl, type, var_name, cap) \
    decl struct                                           \
    {                                                     \
        ArrayHeader_ header;                              \
        type arr[cap];                                    \
    } var_name##_ = {                                     \
        .header = {                                       \
            .length = 0,                                  \
            .capacity = cap,                              \
        },                                                \
        .arr = {0}};                                      \
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

#define ARRAY_MEMORY_SIZE(type, amount) (sizeof(type) * (amount) + sizeof(ArrayHeader_))
#define INIT_ARRAY_MEMORY_FIELD(name, type, amount) _Alignas(ArrayHeader_) uint8_t(name)[ARRAY_MEMORY_SIZE(type, (amount))]

#define GET_ARRAY_HEADER(arr_ptr) ((ArrayHeader_ *)(arr_ptr) - 1)
#define GET_ARRAY_CAPACITY(arr_ptr) (GET_ARRAY_HEADER(arr_ptr)->capacity)
#define GET_ARRAY_LENGTH(arr_ptr) (GET_ARRAY_HEADER(arr_ptr)->length)
#define SET_ARRAY_FIELD_CAPACITY(arr) GET_ARRAY_CAPACITY(arr) = ARRAY_SIZE(arr)

#define ARRAY_FOR(arr_ptr, index_name) \
    for (size_t index_name = 0; index_name < GET_ARRAY_LENGTH(arr_ptr); ++index_name)

#define ARRAY_PUSH_UNCHECKED(arr_ptr, element)            \
    do                                                    \
    {                                                     \
        (arr_ptr)[GET_ARRAY_LENGTH(arr_ptr)] = (element); \
        GET_ARRAY_LENGTH(arr_ptr) += 1;                   \
    } while (0)

#define ARRAY_PUSH_CHECKED(arr_ptr, element, on_err)                  \
    do                                                                \
    {                                                                 \
        if (GET_ARRAY_CAPACITY(arr_ptr) <= GET_ARRAY_LENGTH(arr_ptr)) \
        {                                                             \
            on_err                                                    \
        }                                                             \
        else                                                          \
        {                                                             \
            ARRAY_PUSH_UNCHECKED(arr_ptr, element);                   \
        }                                                             \
    } while (0)

#define ARRAY_PUSH_CHECKED_DEFAULT_RETURN(logger, arr_ptr, element)                  \
    ARRAY_PUSH_CHECKED(arr_ptr, element, {                                           \
        LOG_ERR_TRACE(logger, "Unable to push to " #arr_ptr ", exceeding capacity"); \
        return -1;                                                                   \
    })

#define ARRAY_PUSH_MULTI_CHECKED(arr_ptr, first_element, element_count, on_err)                                   \
    do                                                                                                            \
    {                                                                                                             \
        if (GET_ARRAY_CAPACITY(arr_ptr) < GET_ARRAY_LENGTH(arr_ptr) + (element_count))                            \
        {                                                                                                         \
            on_err                                                                                                \
        }                                                                                                         \
        else                                                                                                      \
        {                                                                                                         \
            memcpy(&(arr_ptr)[GET_ARRAY_LENGTH(arr_ptr)], (first_element), sizeof(*(arr_ptr)) * (element_count)); \
            GET_ARRAY_LENGTH(arr_ptr) += (element_count);                                                         \
        }                                                                                                         \
    } while (0)

#define ARRAY_PUSH_ARRAY_CHECKED(arr_ptr, other_arr_ptr, on_err) \
    ARRAY_PUSH_MULTI_CHECKED(arr_ptr, other_arr_ptr, GET_ARRAY_LENGTH(other_arr_ptr), on_err)

#define ARRAY_PUSH_ARRAY_CHECKED_DEFAULT_RETURN(logger, arr_ptr, other_arr_ptr)            \
    ARRAY_PUSH_ARRAY_CHECKED(arr_ptr, other_arr_ptr, {                                     \
        LOG_ERR_TRACE(logger, "Unable to push array to " #arr_ptr ", exceeding capacity"); \
        return -1;                                                                         \
    })
