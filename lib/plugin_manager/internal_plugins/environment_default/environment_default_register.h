#pragma once

#pragma pack(push, 8)

struct LoggerInterface;

typedef struct EnvironmentInterfaceContext
{
    int argc;
    char **argv;
    void *platform_context;
} EnvironmentInterfaceContext;

struct EnvironmentInterface *environment_get_interface(void);

#pragma pack(pop)