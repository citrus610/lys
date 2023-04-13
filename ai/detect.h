#pragma once

#include "../core/core.h"

namespace Detect
{

struct Score
{
    Chain::Score chain = { 0, 0 };
    i32 needed = 0;
    i32 height = 0;
};

struct Result
{
    Score main = Score();
    Score harass = Score();
};

Result detect(Field& field);

Result detect_deep(Field& field);

bool is_well(u8 heights[6], i8 x);

static inline bool cmp_main(const Score& a, const Score& b)
{
    if (a.chain.count != b.chain.count) {
        return a.chain.count < b.chain.count;
    }
    return a.chain.score < b.chain.score;
};

static inline bool cmp_sub(const Score& a, const Score& b)
{
    if (a.chain.count == 0) return true;
    return a.chain.score * b.chain.count < b.chain.score * a.chain.count;
};

};