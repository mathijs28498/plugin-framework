#pragma once



#pragma pack(push, 8)

struct LoggerInterface;
struct WindowInterface;
struct InputInterface;
struct DrawInterface;

typedef struct GuiApplicationInterfaceContext
{
    struct LoggerInterface *logger;
    struct WindowInterface *window;
    struct InputInterface *input;
    struct DrawInterface *draw;
} GuiApplicationInterfaceContext;

#pragma pack(pop)

