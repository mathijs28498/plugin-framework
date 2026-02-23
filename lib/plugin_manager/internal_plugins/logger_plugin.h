#pragma once

enum LoggerInterfaceLogLevel;
#define LOGGER_PLUGIN_LOG_LEVEL_MAX 4

typedef struct LoggerInterfaceContext
{
    enum LoggerInterfaceLogLevel log_level;
    const char *colors[LOGGER_PLUGIN_LOG_LEVEL_MAX]; 
} LoggerInterfaceContext;

struct LoggerInterface;
struct LoggerInterface *logger_interface_get_interface(void);