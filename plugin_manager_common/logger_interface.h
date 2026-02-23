#pragma once

struct LoggerInterfaceContext;

typedef enum LoggerInterfaceLogLevel
{
    LOG_LEVEL_NONE = -1,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_MAX,
} LoggerInterfaceLogLevel;

#pragma pack(push, 8)

typedef struct LoggerInterface
{
    struct LoggerInterfaceContext *context;

    void (*log)(const struct LoggerInterfaceContext *context, LoggerInterfaceLogLevel log_level, LoggerInterfaceLogLevel urgent_log_level, const char *tag, const char *message, ...);
    void (*set_level)(struct LoggerInterfaceContext *context, LoggerInterfaceLogLevel log_level);
    void (*set_colors)(struct LoggerInterfaceContext *context, const char *[LOG_LEVEL_MAX]);
    void (*on_exit)(struct LoggerInterfaceContext *context);

} LoggerInterface;

#pragma pack(pop)

#define LOGGER_INTERFACE_REGISTER_URGENCY(tag, log_level, urgent_log_level)           \
    static const char LOGGER_INTERFACE_TAG[] = #tag;                                  \
    static const enum LoggerInterfaceLogLevel LOGGER_INTERFACE_LOG_LEVEL = log_level; \
    static const enum LoggerInterfaceLogLevel LOGGER_INTERFACE_URGENT_LOG_LEVEL = urgent_log_level;

#define LOGGER_INTERFACE_REGISTER(tag, log_level) LOGGER_INTERFACE_REGISTER_URGENCY(tag, log_level, LOG_LEVEL_WARNING)

#define LOG(logger, log_level, ...)                                                                                            \
    do                                                                                                                         \
    {                                                                                                                          \
        if (log_level <= LOGGER_INTERFACE_LOG_LEVEL)                                                                           \
        {                                                                                                                      \
            (logger)->log((logger)->context, log_level, LOGGER_INTERFACE_URGENT_LOG_LEVEL, LOGGER_INTERFACE_TAG, __VA_ARGS__); \
        }                                                                                                                      \
    } while (0)

#define LOG_ERR(logger, ...) LOG(logger, LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_WRN(logger, ...) LOG(logger, LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOG_INF(logger, ...) LOG(logger, LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_DBG(logger, ...) LOG(logger, LOG_LEVEL_DEBUG, __VA_ARGS__)
