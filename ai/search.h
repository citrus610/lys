#pragma once

#include "detect.h"
#include "move.h"
#include "eval.h"
#include "ttable.h"

namespace Search
{

struct Attack
{
    i32 score = 0;
    i32 count = 0;
    i32 frame = 0;
    bool all_clear = false;
};

struct Candidate
{
    Move::Placement placement;
    std::vector<Attack> attacks;
    i32 eval = INT32_MIN;
};

struct Result
{
    std::vector<Candidate> candidates;
};

struct Node
{
    Field field = Field();
    i32 score = 0;
    i32 frame = 0;
    i32 eval = INT32_MIN;
};

Result search
(
    Field field,
    std::vector<Cell::Pair> queue,
    Eval::Weight w = Eval::DEFAULT_WEIGHT
);

i32 best_first_search
(
    Field& field,
    std::vector<Cell::Pair>& queue,
    Eval::Weight& w,
    TTable<i32>& table,
    std::vector<Attack>& attacks,
    i32 score,
    i32 frame,
    i32 depth
);

};