#include "logger_console.h"

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <stdbool.h>
#include <conio.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(logger_console, LOG_LEVEL_DEBUG);
#include <plugin_utils.h>

#include "logger_console_register.h" 

STATIC_ASSERT(LOG_LEVEL_MAX == LOGGER_CONSOLE_LOG_LEVEL_MAX, "log_level max_mismatch!");

#define TIME_STRING_LEN sizeof("[00:00:00.000,000]")


const char *LOG_LEVEL_STR_LIST[] = {"ERR", "WRN", "INF", "DBG"};

TODO("Add functionality to allow for time formatting (year/day, no microsecond etc)")
void get_time_str(char time_str[TIME_STRING_LEN])
{
    TODO("Make this a separate time plugin that gets injected into the logger")
    FILETIME ft;
    SYSTEMTIME st_utc, st_local;

    GetSystemTimePreciseAsFileTime(&ft);

    FileTimeToSystemTime(&ft, &st_utc);

    SystemTimeToTzSpecificLocalTime(NULL, &st_utc, &st_local);

    ULONGLONG time_100ns = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;

    int milliseconds = st_local.wMilliseconds;
    int microseconds = (time_100ns % 10000) / 10;

    snprintf(
        time_str,
        TIME_STRING_LEN,
        "[%02d:%02d:%02d.%03d,%03d]",
        st_local.wHour,
        st_local.wMinute,
        st_local.wSecond,
        milliseconds,
        microseconds);
}



void logger_console_log(const LoggerInterfaceContext *context, LoggerInterfaceLogLevel log_level, LoggerInterfaceLogLevel urgent_log_level, const char *tag, const char *message, ...)
{
    if (log_level > context->log_level || log_level <= LOG_LEVEL_NONE || log_level >= LOG_LEVEL_MAX)
    {
        return;
    }

    char time_str[TIME_STRING_LEN];
    get_time_str(time_str);

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

    printf(" %s: ", tag);

    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);

    if (is_urgent)
    {
        printf("%s", ANSI_COLOR_RESET);
    }

    printf("\n");
}

void logger_console_set_level(LoggerInterfaceContext *context, LoggerInterfaceLogLevel log_level)
{
    context->log_level = log_level;
}

void logger_console_set_colors(LoggerInterfaceContext *context, const char *new_colors[LOG_LEVEL_MAX])
{
    for (int i = 0; i < ARRAY_SIZE(new_colors); i++)
    {
        context->colors[i] = new_colors[i];
    }
}

void logger_console_on_program_exit(LoggerInterfaceContext *context, int exit_code)
{
    logger_console_log(context, LOG_LEVEL_INFO, LOGGER_INTERFACE_URGENT_LOG_LEVEL, LOGGER_INTERFACE_TAG, "Program exited with code %d", exit_code);
    logger_console_log(context, LOG_LEVEL_INFO, LOGGER_INTERFACE_URGENT_LOG_LEVEL, LOGGER_INTERFACE_TAG, "Press any key to exit...");
    _getch();
}