#pragma once

#pragma pack(push, 8)

struct LoggerInterface;

typedef struct EnvironmentContext
{
    int argc;
    char **argv;
    void *platform_context;
} EnvironmentContext;

struct EnvironmentInterface *environment_get_interface(void);

#pragma pack(pop)