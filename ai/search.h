#pragma once

#include "layer.h"
#include "path.h"

namespace Search
{

namespace Beam
{

struct Result
{
    Move::Placement placement;
    Field plan = Field();
    i32 eval = INT32_MIN;
};

Result search
(
    Field field,
    std::vector<Cell::Pair> queue,
    Eval::Weight w = Eval::DEFAULT_WEIGHT
);

void expand
(
    Node& node,
    Cell::Pair pair,
    Layer& layer,
    Eval::Weight w
);

};

namespace Attack
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
};

struct Result
{
    std::vector<Candidate> candidates;
};

Result search
(
    Field field,
    std::vector<Cell::Pair> queue,
    i32 thread_count = 4
);

void dfs
(
    Field& field,
    std::vector<Cell::Pair>& queue,
    std::vector<Attack>& attacks,
    i32 score,
    i32 frame,
    i32 depth
);

};

};