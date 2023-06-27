#pragma once

#include "spsa.h"
#include <vector>
#include <fstream>
#include <string>
#include <iomanip>
#include "../lib/nlohmann/json.hpp"
using json = nlohmann::json;
using order_json = nlohmann::ordered_json;

struct Theta
{
    std::vector<double> theta;
    double eval;
};

struct SaveData
{
    std::vector<Theta> data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Theta, theta, eval);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SaveData, data);