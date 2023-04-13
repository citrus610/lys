#pragma once

#include "search.h"
#include "path.h"

namespace AI
{

struct Result
{
    Move::Placement placement = Move::Placement();
    i32 eval = INT32_MIN;
};

struct Enemy
{
    Field field = Field();
    std::vector<Cell::Pair> queue;
    i32 attack = 0;
    i32 attack_frame = 0;
    bool all_clear = false;
};

struct Data
{
    i32 target = 0;
    i32 bonus = 0;
    bool all_clear = false;
};

Result think_1p(Field field, std::vector<Cell::Pair> queue, Eval::Weight w = Eval::DEFAULT_WEIGHT);

Result think_2p(Field field, std::vector<Cell::Pair> queue, Data data, Enemy enemy, Eval::Weight w = Eval::DEFAULT_WEIGHT);

Result build(Search::Attack::Result& result_a, Search::Beam::Result& result_b, u32 field_count);

bool get_enemy_danger(Data& data, Enemy& enemy, Field& field);

};