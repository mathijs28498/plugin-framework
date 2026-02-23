#include "logger_plugin.h"

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <stdbool.h>
#include <conio.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(logger_plugin, LOG_LEVEL_DEBUG);
#include <plugin_manager_common.h>

TODO("Use the known macros for the interface implementation")

STATIC_ASSERT(LOG_LEVEL_MAX == LOGGER_PLUGIN_LOG_LEVEL_MAX, "log_level max_mismatch!");

#define TIME_STRING_LEN sizeof("[00:00:00.000,000]")

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

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

void log(const LoggerInterfaceContext *context, LoggerInterfaceLogLevel log_level, LoggerInterfaceLogLevel urgent_log_level, const char *tag, const char *message, ...)
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

void set_level(LoggerInterfaceContext *context, LoggerInterfaceLogLevel log_level)
{
    context->log_level = log_level;
}

TODO("Check all places that ARRAY_SIZE should be added")
void set_colors(LoggerInterfaceContext *context, const char *new_colors[LOG_LEVEL_MAX])
{
    for (int i = 0; i < ARRAY_SIZE(new_colors); i++)
    {
        context->colors[i] = new_colors[i];
    }
}

LoggerInterface *logger_interface_get_interface(void);

void logger_interface_on_exit(LoggerInterfaceContext *context)
{
    (void)context;
#if IS_DEBUG && WINDOWS_GUI
    LOG_INF(logger_interface_get_interface(), "\nPress any key to exit...\n");
    _getch();
#endif
}

LoggerInterface *logger_interface_get_interface(void)
{
    static LoggerInterfaceContext context = {
        .log_level = LOG_LEVEL_DEBUG,
        .colors = {ANSI_COLOR_RED, ANSI_COLOR_YELLOW, ANSI_COLOR_GREEN, ANSI_COLOR_CYAN},
    };

    static LoggerInterface iface = {
        .context = &context,
        .log = log,
        .set_level = set_level,
        .set_colors = set_colors,
        .on_exit = logger_interface_on_exit,
    };

    TODO("Add this to an init function")
#if IS_DEBUG && WINDOWS_GUI
    if (AllocConsole())
    {
        FILE *fDummy;

        // Redirect standard streams to the new console "CONOUT$" and "CONIN$"
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONIN$", "r", stdin);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE)
        {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode))
            {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
    }
#endif // #if IS_DEBUG && WINDOWS_GUI

    return &iface;
}
