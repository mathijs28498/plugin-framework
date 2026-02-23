#pragma once

#pragma pack(push, 8)

struct RendererInterfaceContext;

typedef struct RendererInterface
{
    struct RendererInterfaceContext *context;
} RendererInterface;

#pragma pack(pop)
