#pragma once



#pragma pack(push, 8)

struct LoggerApi;
struct WindowApi;
struct InputApi;
struct DrawApi;

typedef struct GuiApplicationApiContext
{
    struct LoggerApi *logger_api;
    struct WindowApi *window_api;
    struct InputApi *input_api;
    struct DrawApi *draw_api;
} GuiApplicationApiContext;

#pragma pack(pop)

