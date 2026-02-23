#pragma once

#include <stdint.h>

#pragma pack(push, 8)

struct DrawInterfaceContext;

typedef struct DrawInterface
{
    struct DrawInterfaceContext *context;

    int32_t (*present)(struct DrawInterfaceContext *context);
} DrawInterface;

#pragma pack(pop)
