#pragma once

struct TimeContext;

#define TIME_WIN32_STRING_LEN sizeof("[00:00:00.000,000]")
void time_win32_get_string(struct TimeContext* context, char time_str[TIME_WIN32_STRING_LEN]);