#pragma once
#ifndef TEST_API_2_H
#define TEST_API_2_H

#include <stdint.h>

#pragma pack(push, 8)

struct TestApi2Context;

typedef struct TestApi2
{
    struct TestApi2Context *context;

    int32_t (*add)(struct TestApi2Context *context, int32_t a, int32_t b);
} TestApi2;

#pragma pack(pop)

#endif // #ifndef TEST_API_2_H