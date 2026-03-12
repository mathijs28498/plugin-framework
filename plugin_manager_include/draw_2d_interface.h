#pragma once

#pragma pack(push, 8)

struct Draw2dInterfaceContext;

typedef struct Draw2dInterface
{
    struct Draw2dInterfaceContext *context;

    void (*test)(struct Draw2dInterfaceContext *context, int test_int);
} Draw2dInterface;

#pragma pack(pop)
