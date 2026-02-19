#pragma once

#include <stdint.h>

#pragma pack(push, 8)

struct DrawApiContext;

typedef struct DrawApi
{
    struct DrawApiContext *context;

    int32_t (*present)(struct DrawApiContext *context);
} DrawApi;

#pragma pack(pop)
