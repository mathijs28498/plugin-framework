#pragma once
#ifndef LOGGER_API_H
#define LOGGER_API_H

struct LoggerApiContext;

typedef enum LoggerApiLogLevel
{
    LOG_LEVEL_NONE = -1,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_MAX,
} LoggerApiLogLevel;

#pragma pack(push, 8)

typedef struct LoggerApi
{
    struct LoggerApiContext *context;

    void (*log)(const struct LoggerApiContext *context, LoggerApiLogLevel log_level, LoggerApiLogLevel urgent_log_level, const char *tag, const char *message, ...);
    void (*set_level)(struct LoggerApiContext *context, LoggerApiLogLevel log_level);
    void (*set_colors)(struct LoggerApiContext *context, const char *[LOG_LEVEL_MAX]);

} LoggerApi;

#pragma pack(pop)

#define LOGGER_API_REGISTER_URGENCY(tag, log_level, urgent_log_level)     \
    static const char LOGGER_API_TAG[] = #tag;                            \
    static const enum LoggerApiLogLevel LOGGER_API_LOG_LEVEL = log_level; \
    static const enum LoggerApiLogLevel LOGGER_API_URGENT_LOG_LEVEL = urgent_log_level;

#define LOGGER_API_REGISTER(tag, log_level) LOGGER_API_REGISTER_URGENCY(tag, log_level, LOG_LEVEL_WARNING)

#define LOG(logger_api, log_level, ...)                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        if (log_level <= LOGGER_API_LOG_LEVEL)                                                                         \
        {                                                                                                              \
            (logger_api)->log((logger_api)->context, log_level, LOGGER_API_URGENT_LOG_LEVEL, LOGGER_API_TAG, __VA_ARGS__); \
        }                                                                                                              \
    } while (0)

#define LOG_ERR(logger_api, ...) LOG(logger_api, LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_WRN(logger_api, ...) LOG(logger_api, LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOG_INF(logger_api, ...) LOG(logger_api, LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_DBG(logger_api, ...) LOG(logger_api, LOG_LEVEL_DEBUG, __VA_ARGS__)

#endif // #ifndef LOGGER_API_H