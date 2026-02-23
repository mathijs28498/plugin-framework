#pragma once

#include <stdint.h>

struct LoggerInterface;
struct PluginRegistry;

int32_t plugin_registry_deserialize_json(struct LoggerInterface *logger, const char *json_str, struct PluginRegistry *plugin_registry);