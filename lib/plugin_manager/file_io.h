
#pragma once
#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdint.h>
#include <plugin_manager_common.h>
TODO("Make this a system plugin")

int32_t file_io_read(const char *path, char **buffer_out);

#endif // #ifndef FILE_IO_H