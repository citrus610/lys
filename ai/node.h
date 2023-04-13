#pragma once

#include "detect.h"
#include "move.h"
#include "eval.h"

struct Node
{
    Field field = Field();
    i32 frame = 0;
    i32 index = -1;
    i32 eval = INT32_MIN;
};

static inline bool operator < (const Node& a, const Node& b)
{
    return a.eval < b.eval;
};