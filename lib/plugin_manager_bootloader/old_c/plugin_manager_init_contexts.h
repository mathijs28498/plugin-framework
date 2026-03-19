#pragma once

#include <stdint.h>

struct PluginManagerSetupContext;
struct PluginManagerRuntimeContext;

int32_t plugin_manager_init_contexts(struct PluginManagerSetupContext **setup_context, struct PluginManagerRuntimeContext **runtime_context);