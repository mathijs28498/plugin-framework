#pragma once

#pragma pack(push, 8)

struct LoggerApi;

typedef struct RendererApiContext
{
    struct LoggerApi *logger_api;
} RendererApiContext;

#pragma pack(pop)