#pragma once

#include "layer.h"
#include "move.h"
#include "eval.h"
#include "detect.h"

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

void init_candidates(
    std::vector<Candidate>& candidates,
    std::vector<Cell::Pair>& queue,
    Field& root
);

void search_attack(
    std::vector<Candidate>& candidates,
    std::vector<Cell::Pair>& queue
);

void search_beam(
    std::vector<Candidate>& candidates,
    std::vector<Cell::Pair>& queue
);

};