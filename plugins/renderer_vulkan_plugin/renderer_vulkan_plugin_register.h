#pragma once

#pragma pack(push, 8)

struct LoggerInterface;

typedef struct RendererInterfaceContext
{
    struct LoggerInterface *logger;
} RendererInterfaceContext;

#pragma pack(pop)