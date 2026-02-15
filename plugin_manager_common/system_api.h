#pragma once
#ifndef SYSTEM_API_H
#define SYSTEM_API_H

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARN = 2,
    LOG_ERROR = 3,
} LogLevel;


typedef void (*LogFunc)(LogLevel level, const char * module, const char *fmt, ...);

typedef struct {
    LogFunc log;
} SystemApi;

#ifndef LOG_LEVEL
#define LOG_LEVEL -1
#endif // #ifndef LOG_LEVEL


#if LOG_LEVEL <= LOG_DEBUG
    #define LOG_DBG(logger, module, fmt, ...) \
        (logger(LOG_DEBUG, module, fmt, ##__VA_ARGS__))
#else
    #define LOG_DBG(logger, module, fmt, ...) ((void)0)
#endif

#if LOG_LEVEL <= LOG_INFO
    #define LOG_INFO(logger, module, fmt, ...) \
        (logger(LOG_INFO, module, fmt, ##__VA_ARGS__))
#else
    #define LOG_INFO(logger, module, fmt, ...) ((void)0)
#endif

#if LOG_LEVEL <= LOG_WARN
    #define LOG_WARN(logger, module, fmt, ...) \
        (logger(LOG_WARN, module, fmt, ##__VA_ARGS__))
#else
    #define LOG_WARN(logger, module, fmt, ...) ((void)0)
#endif

#if LOG_LEVEL <= LOG_ERROR
    #define LOG_ERR(logger, module, fmt, ...) \
        (logger(LOG_ERROR, module, fmt, ##__VA_ARGS__))
#else
    #define LOG_ERR(logger, module, fmt, ...) ((void)0)
#endif

#endif // #ifndef SYSTEM_API_H