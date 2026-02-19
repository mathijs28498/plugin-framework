#pragma once

#pragma pack(push, 8)

struct RendererApiContext;

typedef struct RendererApi
{
    struct RendererApiContext *context;
} RendererApi;

#pragma pack(pop)
