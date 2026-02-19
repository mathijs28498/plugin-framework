#pragma once



#include <stdbool.h>

#pragma pack(push, 8)

struct LoggerApi;

typedef struct TestApi2Context
{
    struct LoggerApi *logger_api;

    bool is_initialized;
} TestApi2Context;

#pragma pack(pop)

