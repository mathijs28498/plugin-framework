#include "logger_console_register.h"

#include <plugin_utils.h>

#include <stdint.h>
#include <stdbool.h>
TODO("Remove this windows include")
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(logger_console, LOG_LEVEL_DEBUG);

#include "logger_console.h"

STATIC_ASSERT(LOG_LEVEL_MAX == LOGGER_CONSOLE_LOG_LEVEL_MAX, "log_level max_mismatch!");

static const LoggerVtable plugin_vtable = {
    .log = logger_console_log,
    .set_colors = logger_console_set_colors,
    .set_level = logger_console_set_level,
};

static int32_t plugin_init(LoggerContext *context)
{
    context->log_level = LOG_LEVEL_DEBUG;
    context->colors[0] = ANSI_COLOR_RED;
    context->colors[1] = ANSI_COLOR_YELLOW;
    context->colors[2] = ANSI_COLOR_GREEN;
    context->colors[3] = ANSI_COLOR_CYAN;

    if (!AttachConsole(ATTACH_PARENT_PROCESS))
    {
        if (!AllocConsole())
        {
            return 0;
        }
    }

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
    return 0;
}

static int32_t plugin_shutdown(LoggerContext *context)
{
    logger_console_log(context, LOG_LEVEL_INFO, LOGGER_INTERFACE_URGENT_LOG_LEVEL, LOGGER_INTERFACE_TAG, "Press any key to exit...");
    _getch();

    fflush(stdout);
    fflush(stderr);

    fclose(stdout);
    fclose(stderr);
    fclose(stdin);

    FreeConsole();
    return 0;
}

#include "plugin_register.c.inc"