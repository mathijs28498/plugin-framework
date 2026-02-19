#pragma once


#include <stdint.h>

#pragma pack(push, 8)

struct TestApiContext;

typedef struct TestApi
{
    struct TestApiContext *context;

    int32_t (*do_something)(struct TestApiContext *context);
} TestApi;

#pragma pack(pop)

