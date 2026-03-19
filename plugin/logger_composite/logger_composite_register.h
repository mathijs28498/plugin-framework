#pragma once

#include "plugin_dependencies.h"

#pragma pack(push, 8)

struct LoggerInterface;

typedef struct LoggerContext
{
    PLUGIN_CONTEXT_DEPENDENCIES
} LoggerContext;

#pragma pack(pop)

