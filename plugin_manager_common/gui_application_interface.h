#pragma once



#include <stdint.h>

#pragma pack(push, 8)

struct GuiApplicationInterfaceContext;

typedef struct GuiApplicationInterface {
    struct GuiApplicationInterfaceContext *context;

    int32_t (*run)(struct GuiApplicationInterfaceContext *context);
} GuiApplicationInterface;

#pragma pack(pop)

