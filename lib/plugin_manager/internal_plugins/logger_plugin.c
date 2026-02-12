#include "logger_plugin.h"

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

#include <logger_api.h>
#include <plugin_manager_common.h>

#define TIME_STRING_LEN sizeof("[00:00:00.000]")

void get_time_str(char time_str[TIME_STRING_LEN])
{
    TODO("Figure out a way to get the time for each platform with a fallback for just hr, min, sec")
    SYSTEMTIME time;
    GetLocalTime(&time);

    snprintf(
        time_str,
        TIME_STRING_LEN,
        "[%02d:%02d:%02d.%03d]",
        time.wHour,
        time.wMinute,
        time.wSecond,
        time.wMilliseconds);
}

void log(const LoggerApiContext *context, LoggerApiLogLevel log_level, const char *tag, const char *message, ...)
{
    if (log_level > context->log_level)
    {
        return;
    }

    char time_str[TIME_STRING_LEN];
    get_time_str(time_str);

    const char *log_level_str_list[] = {"ERR", "WRN", "INF", "DBG"};
    printf("%s <%s> %s: ",
           time_str,
           log_level_str_list[log_level],
           tag);

    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);

    printf("\n");
}

void set_level(LoggerApiContext *context, LoggerApiLogLevel log_level)
{
    context->log_level = log_level;
}

LoggerApi *logger_api_get_api(void)
{
    static LoggerApiContext context = {
        .log_level = LOG_LEVEL_DEBUG,
    };

    static LoggerApi api = {
        .context = &context,
        .log = log,
        .set_level = set_level,
    };

    return &api;
}