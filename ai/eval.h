#pragma once

#include "../core/core.h"
#include "detect.h"
#include <fstream>
#include <string>
#include <iomanip>
#include "../lib/nlohmann/json.hpp"
using json = nlohmann::json;
using order_json = nlohmann::ordered_json;

namespace Eval
{

struct Weight
{
    i32 diff = 0;
    i32 diff_s = 0;
    i32 shape = 0;
    i32 shape_s = 0;
    i32 nuisance = 0;
    i32 side_bias = 0;
    i32 disconnect = 0;

    i32 frame = 0;

    i32 chain_count = 0;
    i32 chain_score = 0;
    i32 chain_height = 0;
    i32 chain_needed = 0;
};

i32 evaluate(Field& field, std::optional<Detect::Result> detect, u8 frame, Weight& w);

constexpr Weight DEFAULT_WEIGHT = {
    .diff = -50,
    .diff_s = -10,
    .shape = -50,
    .shape_s = -50,
    .nuisance = -100,
    .side_bias = -20,
    .disconnect = -20,

    .frame = -20,

    .chain_count = 500,
    .chain_score = 10,
    .chain_height = 20,
    .chain_needed = -50,
};

constexpr Weight FAST_WEIGHT = {
    .diff = 0,
    .diff_s = 0,
    .shape = 0,
    .shape_s = 0,
    .nuisance = -100,
    .side_bias = -20,
    .disconnect = -100,

    .frame = -100,

    .chain_count = 1000,
    .chain_score = 10,
    .chain_height = 20,
    .chain_needed = -20
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    diff,
    diff_s,
    shape,
    shape_s,
    nuisance,
    side_bias,
    disconnect,
    frame,
    chain_count,
    chain_score,
    chain_height,
    chain_needed
)

};