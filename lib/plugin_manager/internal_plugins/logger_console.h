#pragma once

enum LoggerInterfaceLogLevel;
#define LOGGER_CONSOLE_LOG_LEVEL_MAX 4

typedef struct LoggerInterfaceContext
{
    enum LoggerInterfaceLogLevel log_level;
    const char *colors[LOGGER_CONSOLE_LOG_LEVEL_MAX]; 
} LoggerInterfaceContext;

struct LoggerInterface;
struct LoggerInterface *logger_interface_get_interface(void);
void logger_interface_on_program_exit(LoggerInterfaceContext *context);