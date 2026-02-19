#pragma once

#pragma pack(push, 8)

struct LoggerApi;

typedef struct DrawApiContext
{
    struct LoggerApi *logger_api;
} DrawApiContext;

#pragma pack(pop)