#pragma once

#include "../core/core.h"
#include "detect.h"
#include "form.h"
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
    i32 shape = 0;
    i32 shape_s = 0;
    i32 nuisance = 0;
    i32 side_bias = 0;
    i32 disconnect = 0;
    i32 link_hor = 0;
    i32 link_ver = 0;
    i32 form = 0;

    i32 frame = 0;

    i32 chain_count = 0;
    i32 chain_score = 0;
    i32 chain_height = 0;
    i32 chain_needed = 0;
};

i32 evaluate(Field& field, std::optional<Detect::Result> detect, u8 frame, Weight& w);

constexpr Weight DEFAULT_WEIGHT = {
    .shape = -100,
    .shape_s = 0,
    .nuisance = -50,
    .side_bias = 0,
    .disconnect = 0,
    .link_hor = 10,
    .link_ver = -10,
    .form = 0,

    .frame = -350,

    .chain_count = 1000,
    .chain_score = 0,
    .chain_height = 200,
    .chain_needed = -200,
};

constexpr Weight FAST_WEIGHT = {
    .shape = 0,
    .shape_s = 0,
    .nuisance = -50,
    .side_bias = -20,
    .disconnect = 0,
    .link_hor = 200,
    .link_ver = -200,
    .form = 0,

    .frame = -100,

    .chain_count = 1000,
    .chain_score = 0,
    .chain_height = 100,
    .chain_needed = -100
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    shape,
    shape_s,
    nuisance,
    side_bias,
    disconnect,
    link_hor,
    link_ver,
    form,
    frame,
    chain_count,
    chain_score,
    chain_height,
    chain_needed
)

};