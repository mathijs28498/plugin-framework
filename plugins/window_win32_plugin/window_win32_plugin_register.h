#pragma once
#ifndef WINDOW_WIN32_PLUGIN_REGISTER_H
#define WINDOW_WIN32_PLUGIN_REGISTER_H

#pragma pack(push, 8)

struct LoggerApi;
struct EnvironmentApi;

typedef struct WindowApiContext {
    struct LoggerApi *logger_api;
    struct EnvironmentApi *environment_api;
} WindowApiContext;

#pragma pack(pop)

#endif // #ifndef WINDOW_WIN32_PLUGIN_REGISTER_H