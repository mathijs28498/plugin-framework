#pragma once

#pragma pack(push, 8)

struct Draw2dInterfaceContext;

typedef struct Draw2dInterface
{
    struct Draw2dInterfaceContext *context;
} Draw2dInterface;

#pragma pack(pop)
