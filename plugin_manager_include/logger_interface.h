#pragma once

struct LoggerContext;

typedef enum LoggerInterfaceLogLevel
{
    LOG_LEVEL_NONE = -1,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_MAX,
} LoggerInterfaceLogLevel;

typedef struct LoggerVtable
{
    void (*log)(const struct LoggerContext *context, LoggerInterfaceLogLevel log_level, LoggerInterfaceLogLevel urgent_log_level, const char *tag, const char *message, ...);
    void (*set_level)(struct LoggerContext *context, LoggerInterfaceLogLevel log_level);
    void (*set_colors)(struct LoggerContext *context, const char *colors[LOG_LEVEL_MAX]);
}
LoggerVtable;

#pragma pack(push, 8)

typedef struct LoggerInterface
{
    struct LoggerContext *context;
    LoggerVtable *vtable;
} LoggerInterface;

#pragma pack(pop)

static inline void logger_set_level(LoggerInterface *iface, LoggerInterfaceLogLevel log_level)
{
    iface->vtable->set_level(iface->context, log_level);
}

static inline void logger_set_colors(LoggerInterface *iface, const char *colors[LOG_LEVEL_MAX])
{
    iface->vtable->set_colors(iface->context, colors);
}

#define LOGGER_INTERFACE_REGISTER_URGENCY(tag, log_level, urgent_log_level)           \
    static const char LOGGER_INTERFACE_TAG[] = #tag;                                  \
    static const enum LoggerInterfaceLogLevel LOGGER_INTERFACE_LOG_LEVEL = log_level; \
    static const enum LoggerInterfaceLogLevel LOGGER_INTERFACE_URGENT_LOG_LEVEL = urgent_log_level;

#define LOGGER_INTERFACE_REGISTER(tag, log_level) LOGGER_INTERFACE_REGISTER_URGENCY(tag, log_level, LOG_LEVEL_WARNING)

#define LOG(logger, log_level, ...)                                                                                                    \
    do                                                                                                                                 \
    {                                                                                                                                  \
        if (log_level <= LOGGER_INTERFACE_LOG_LEVEL)                                                                                   \
        {                                                                                                                              \
            (logger)->vtable->log((logger)->context, log_level, LOGGER_INTERFACE_URGENT_LOG_LEVEL, LOGGER_INTERFACE_TAG, __VA_ARGS__); \
        }                                                                                                                              \
    } while (0)

#define LOG_ERR(logger, ...) LOG(logger, LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_WRN(logger, ...) LOG(logger, LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOG_INF(logger, ...) LOG(logger, LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_DBG(logger, ...) LOG(logger, LOG_LEVEL_DEBUG, __VA_ARGS__)
