#pragma once

#include <vector>
#include <fstream>
#include <string>
#include <iomanip>
#include "../lib/nlohmann/json.hpp"
using json = nlohmann::json;
using order_json = nlohmann::ordered_json;

#include "../ai/ai.h"

namespace Eval
{

struct Gene
{
    Weight heuristic = DEFAULT_WEIGHT;
    double score = 0.0;
};

struct SaveData
{
    std::vector<Gene> data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Gene, heuristic, score)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SaveData, data);

};