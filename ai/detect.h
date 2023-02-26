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
    Score flat = Score();
    Score deep = Score();
};

Result detect(Field& field);

bool is_well(u8 heights[6], i8 x);

static inline bool operator < (const Score& a, const Score& b) {
    if (a.chain.count != b.chain.count) {
        return a.chain.count < b.chain.count;
    }
    if (a.height != b.height) {
        return a.height < b.height;
    }
    return a.chain.score < b.chain.score;
};

static inline bool operator == (const Score& a, const Score& b) {
    return a.chain.score == b.chain.score && a.chain.count == b.chain.count;
};

};