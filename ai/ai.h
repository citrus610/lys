#pragma once

#include "search.h"
#include "path.h"

namespace AI
{

enum DangerLevel
{
    DANGER_NONE,
    DANGER_LOW,
    DANGER_MEDIUM,
    DANGER_HIGH
};

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

Result build(Search::Result& search_result, u32 field_count);

i32 get_unburied_count(Field field);

i32 get_attack(Field field, std::vector<Cell::Pair> queue);

i32 get_enemy_danger(Data& data, Enemy& enemy, Field& field);

};