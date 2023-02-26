#pragma once

#include "search.h"

namespace AI
{

struct Result
{
    Move::Placement placement = Move::Placement();
    i32 eval = INT32_MIN;
};

Result think_1p(Field field, std::vector<Cell::Pair> queue, Eval::Weight w = Eval::DEFAULT_WEIGHT);

Result build(Search::Result& search_result, u32 field_count);

};