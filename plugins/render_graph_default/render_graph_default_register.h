#pragma once

#include "plugin_dependencies.h"

#pragma pack(push, 8)

typedef struct RenderGraphContext
{
    PluginDependencies deps;

    struct RG_Pass *passes_a;
} RenderGraphContext;

#pragma pack(pop)

