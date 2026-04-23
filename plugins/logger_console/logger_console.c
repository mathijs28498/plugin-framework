#include "logger_console.h"
#include <plugin_sdk/plugin_utils.h>

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <conio.h>
#include <string.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(logger_console, LOG_LEVEL_DEBUG);
#include <plugin_sdk/time/v1/time_interface.h>

#include "logger_console_register.h"

_Static_assert(LOG_LEVEL_MAX == LOGGER_CONSOLE_LOG_LEVEL_MAX, "log_level max_mismatch!");
const char *LOG_LEVEL_STR_LIST[] = {"ERR", "WRN", "INF", "DBG"};

static const char *get_short_filename(const char *file_path)
{
    assert(file_path != NULL);
    const char *slash = strrchr(file_path, '/');
    const char *backslash = strrchr(file_path, '\\');
    const char *last_slash = slash > backslash ? slash : backslash;
    return last_slash != NULL ? last_slash + 1 : file_path;
}

void log_internal(const LoggerContext *context, LoggerInterfaceLogLevel log_level, LoggerInterfaceLogLevel urgent_log_level, const char *tag, const char *file_path, uint32_t line_number, const char *message, va_list args)
{
    if (log_level > context->log_level || log_level <= LOG_LEVEL_NONE || log_level >= LOG_LEVEL_MAX)
    {
        return;
    }

    char time_str[TIME_STRING_LEN];
    time_get_string(context->deps.time, time_str);

    const char *log_level_str = LOG_LEVEL_STR_LIST[log_level];

    bool is_urgent = log_level <= urgent_log_level;
    const char *log_color_str = context->colors[log_level];
    printf("%s %s<%s>",
           time_str,
           log_color_str,
           log_level_str);

    if (!is_urgent)
    {
        printf("%s", ANSI_COLOR_RESET);
    }

    if (file_path != NULL)
    {
        printf(" [%s:%d]", get_short_filename(file_path), line_number);
    }

    printf(" %s: ", tag);

    vprintf(message, args);

    if (is_urgent)
    {
        printf("%s", ANSI_COLOR_RESET);
    }

    printf("\n");
}

void logger_console_log_trace(const LoggerContext *context, LoggerInterfaceLogLevel log_level, LoggerInterfaceLogLevel urgent_log_level, const char *tag, const char *file_path, uint32_t line_number, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    log_internal(context, log_level, urgent_log_level, tag, file_path, line_number, message, args);
    va_end(args);
}

void logger_console_log(const LoggerContext *context, LoggerInterfaceLogLevel log_level, LoggerInterfaceLogLevel urgent_log_level, const char *tag, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    log_internal(context, log_level, urgent_log_level, tag, NULL, 0, message, args);
    va_end(args);
}

void logger_console_set_level(LoggerContext *context, LoggerInterfaceLogLevel log_level)
{
    context->log_level = log_level;
}

void logger_console_set_colors(LoggerContext *context, const char *new_colors[LOG_LEVEL_MAX])
{
    for (int i = 0; i < ARRAY_SIZE(new_colors); i++)
    {
        context->colors[i] = new_colors[i];
    }
}