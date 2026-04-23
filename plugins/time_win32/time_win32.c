#include "time_win32.h"

#include <Windows.h>
#include <assert.h>
#include <stdio.h>

#include <plugin_sdk/logger/v1/logger_interface.h>
#include <plugin_sdk/logger/v1/logger_interface_macros.h>
LOGGER_INTERFACE_REGISTER(time_win32, LOG_LEVEL_DEBUG)
#include <plugin_sdk/time/v1/time_interface.h>

#include "time_win32_register.h"

_Static_assert(TIME_WIN32_STRING_LEN == TIME_STRING_LEN, "Time string length of time_win32 doesn't match interface");

void time_win32_get_string(TimeContext *context, char time_str[TIME_WIN32_STRING_LEN])
{
    (void) context;
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