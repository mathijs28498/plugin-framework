#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if PLUGIN_BUILD_SHARED
struct LoggerInterface;
struct RequestedPlugin;

int32_t plugin_manager_request_plugin(
    const struct LoggerInterface *logger,
    const char *interface_name,
    const char *plugin_name,
    bool is_explicit,
    struct RequestedPlugin *requested_plugins,
    size_t *requested_plugins_len);
#endif // #if PLUGIN_BUILD_SHARED