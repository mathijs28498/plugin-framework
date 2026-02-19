
#pragma once

#include <stdint.h>
#include <plugin_manager_common.h>

TODO("Make this a system plugin")

struct LoggerApi;

int32_t file_io_read(struct LoggerApi *logger_api, const char *path, char **buffer_out);