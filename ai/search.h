#pragma once

#include "detect.h"
#include "move.h"
#include "eval.h"

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
    i32 eval_fast = INT32_MIN;
};

struct Result
{
    std::vector<Candidate> candidates;
};

Result search
(
    Field field,
    std::vector<Cell::Pair> queue,
    Eval::Weight w = Eval::DEFAULT_WEIGHT,
    i32 thread_count = 4
);

i32 bfs
(
    Field& field,
    std::vector<Cell::Pair>& queue,
    Eval::Weight& w,
    std::vector<Attack>& attacks,
    i32 score,
    i32 frame,
    i32 depth
);

Result search_attacks
(
    Field field,
    std::vector<Cell::Pair> queue,
    bool fast = true,
    i32 thread_count = 4
);

void bfs_attacks
(
    Field& field,
    std::vector<Cell::Pair>& queue,
    std::vector<Attack>& attacks,
    bool fast,
    i32 score,
    i32 frame,
    i32 depth
);

};