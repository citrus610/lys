#pragma once

#include "../core/core.h"

struct Node
{
    Field field = Field();
    i32 eval = INT32_MIN;
    i32 index = -1;
};

static bool operator < (const Node& a, const Node& b)
{
    return a.eval < b.eval;
};