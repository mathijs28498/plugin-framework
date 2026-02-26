#include "logger_console_register.h"

#include <plugin_utils.h>

#include <stdint.h>
#include <stdbool.h>
TODO("Remove this windows include")
#include <windows.h>
#include <stdio.h>

#include <logger_interface.h>
#include <plugin_sdk.h>

#include "logger_console.h"

STATIC_ASSERT(LOG_LEVEL_MAX == LOGGER_CONSOLE_LOG_LEVEL_MAX, "log_level max_mismatch!");

LoggerInterface *get_interface(void)
{
    static LoggerInterfaceContext context = {
        .log_level = LOG_LEVEL_DEBUG,
        .colors = {ANSI_COLOR_RED, ANSI_COLOR_YELLOW, ANSI_COLOR_GREEN, ANSI_COLOR_CYAN},
    };

    static LoggerInterface iface = {
        .context = &context,

        .log = logger_console_log,
        .set_colors = logger_console_set_colors,
        .set_level = logger_console_set_level,
    };

    TODO("Add this to an init function")
    return &iface;
}

TODO("Figure out how to make this work with the macro")
PLUGIN_REGISTER_INTERFACE(get_interface, LoggerInterface);

int32_t init(LoggerInterfaceContext *context)
{
    (void)context;

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
    return 0;
}

PLUGIN_REGISTER_INIT(init, LoggerInterfaceContext)