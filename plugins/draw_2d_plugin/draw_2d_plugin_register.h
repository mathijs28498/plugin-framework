#pragma once

#pragma pack(push, 8)

struct LoggerApi;

typedef struct Draw2dApiContext
{
    struct LoggerApi *logger_api;
} Draw2dApiContext;

#pragma pack(pop)