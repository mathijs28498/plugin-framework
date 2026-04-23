#include "logger_interface.h"

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

#define LOG_TRACE(logger, log_level, ...)                                                                                                                        \
    do                                                                                                                                                           \
    {                                                                                                                                                            \
        if (log_level <= LOGGER_INTERFACE_LOG_LEVEL)                                                                                                             \
        {                                                                                                                                                        \
            (logger)->vtable->log_trace((logger)->context, log_level, LOGGER_INTERFACE_URGENT_LOG_LEVEL, LOGGER_INTERFACE_TAG, __FILE__, __LINE__, __VA_ARGS__); \
        }                                                                                                                                                        \
    } while (0)

#define LOG_ERR_TRACE(logger, ...) LOG_TRACE(logger, LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_WRN_TRACE(logger, ...) LOG_TRACE(logger, LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOG_INF_TRACE(logger, ...) LOG_TRACE(logger, LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_DBG_TRACE(logger, ...) LOG_TRACE(logger, LOG_LEVEL_DEBUG, __VA_ARGS__)
