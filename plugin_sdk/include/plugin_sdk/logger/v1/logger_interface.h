#pragma once

#include <stdint.h>

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
    void (*log_trace)(const struct LoggerContext *context, LoggerInterfaceLogLevel log_level, LoggerInterfaceLogLevel urgent_log_level, const char *tag, const char *file_path, uint32_t line_number, const char *message, ...);
    void (*set_level)(struct LoggerContext *context, LoggerInterfaceLogLevel log_level);
    void (*set_colors)(struct LoggerContext *context, const char *colors[LOG_LEVEL_MAX]);
} LoggerVtable;

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
