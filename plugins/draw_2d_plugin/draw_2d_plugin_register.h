#pragma once

#pragma pack(push, 8)

struct LoggerInterface;

typedef struct Draw2dInterfaceContext
{
    struct LoggerInterface *logger;
} Draw2dInterfaceContext;

#pragma pack(pop)