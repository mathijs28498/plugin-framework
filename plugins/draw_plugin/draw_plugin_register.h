#pragma once

#pragma pack(push, 8)

struct LoggerInterface;

typedef struct DrawInterfaceContext
{
    struct LoggerInterface *logger;
} DrawInterfaceContext;

#pragma pack(pop)