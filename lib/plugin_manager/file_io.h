
#pragma once

#include <stdint.h>
#include <plugin_manager_common.h>

TODO("Make this a system plugin")

struct LoggerInterface;

int32_t file_io_read(struct LoggerInterface *logger, const char *path, char **buffer_out);