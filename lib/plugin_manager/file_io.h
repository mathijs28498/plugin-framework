
#pragma once

#include <stdint.h>
#include <plugin_utils.h>

TODO("Make this a system plugin")

struct LoggerInterface;

int32_t file_io_read(struct LoggerInterface *logger, const char *path, char **buffer_out);