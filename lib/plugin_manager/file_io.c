#include "file_io.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <plugin_manager_common.h>
#include <logger_api.h>
LOGGER_API_REGISTER(file_io, LOG_LEVEL_DEBUG)

TODO("Replace malloc")
int32_t file_io_read(LoggerApi *logger_api, const char *path, char **buffer_out)
{
    FILE *system_json_file;
    int ret;
    ret = fopen_s(&system_json_file, path, "rb");
    LOG_DBG(logger_api, "After fopen_s '%d', '%p'", ret, system_json_file);

    TODO("Do this without malloc (get a define with the size of the file)")
    fseek(system_json_file, 0, SEEK_END);
    size_t length = ftell(system_json_file);
    fseek(system_json_file, 0, SEEK_SET);
    // Reserve 1 byte for the null terminator
    *buffer_out = malloc(length + 1);
    if (*buffer_out)
    {
        fread(*buffer_out, 1, length, system_json_file);
        (*buffer_out)[length] = '\0';
    }
    fclose(system_json_file);

    return 0;
}