#pragma once

enum LoggerApiLogLevel;
#define LOGGER_PLUGIN_LOG_LEVEL_MAX 4

typedef struct LoggerApiContext
{
    enum LoggerApiLogLevel log_level;
    const char *colors[LOGGER_PLUGIN_LOG_LEVEL_MAX]; 
} LoggerApiContext;

struct LoggerApi;
struct LoggerApi *logger_api_get_api(void);