#pragma once

#pragma pack(push, 8)

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

enum LoggerInterfaceLogLevel;
#define LOGGER_CONSOLE_LOG_LEVEL_MAX 4

typedef struct LoggerContext
{
    enum LoggerInterfaceLogLevel log_level;
    const char *colors[LOGGER_CONSOLE_LOG_LEVEL_MAX]; 
} LoggerContext;

struct LoggerInterface *logger_get_interface(void);

#pragma pack(pop)