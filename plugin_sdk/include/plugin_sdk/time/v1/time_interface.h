#pragma once

#include "../../plugin_utils.h"

#pragma pack(push, 8)

struct TimeContext;

#define TIME_STRING_LEN sizeof("[00:00:00.000,000]")

TODO("Add functionality to allow for time formatting (year/day, no microsecond etc)")

typedef struct TimeVtable
{
    void (*get_string)(struct TimeContext *context, char time_str[TIME_STRING_LEN]);
} TimeVtable;

typedef struct TimeInterface
{
    struct TimeContext *context;
    TimeVtable *vtable;
} TimeInterface;

#pragma pack(pop)

inline void time_get_string(TimeInterface *iface, char time_str[TIME_STRING_LEN])
{
    iface->vtable->get_string(iface->context, time_str);
}