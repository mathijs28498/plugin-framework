#pragma once



#include <stdint.h>

#pragma pack(push, 8)

struct GuiApplicationApiContext;

typedef struct GuiApplicationApi {
    struct GuiApplicationApiContext *context;

    int32_t (*run)(struct GuiApplicationApiContext *context);
} GuiApplicationApi;

#pragma pack(pop)

