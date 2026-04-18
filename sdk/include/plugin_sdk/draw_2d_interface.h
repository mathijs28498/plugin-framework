#pragma once

#pragma pack(push, 8)

struct Draw2dContext;

typedef struct Draw2dVtable
{
    void (*test)(struct Draw2dContext *context, int test_int);
} Draw2dVtable;

typedef struct Draw2dInterface
{
    struct Draw2dContext *context;
    Draw2dVtable *vtable;
} Draw2dInterface;

#pragma pack(pop)
