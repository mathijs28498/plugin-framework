#pragma once

#pragma pack(push, 8)

struct Draw2dApiContext;

typedef struct Draw2dApi
{
    struct Draw2dApiContext *context;
} Draw2dApi;

#pragma pack(pop)
