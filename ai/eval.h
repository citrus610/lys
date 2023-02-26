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
    i32 link_v = 0;
    i32 link_h = 0;
    i32 link_mid = 0;
    i32 d_height = 0;
    i32 d_height_sq = 0;
    i32 symm = 0;
    i32 symm_sq = 0;
    i32 shape_u = 0;
    i32 shape_u_sq = 0;

    i32 frame = 0;

    i32 ptnl_chain_count = 0;
    i32 ptnl_chain_score = 0;
    i32 ptnl_chain_needed = 0;
    i32 ptnl_chain_height = 0;
};

i32 evaluate(Field& field, std::optional<Detect::Score> detect, u8 frame, Weight& w);

void link(Field& field, i32& link_v, i32& link_h, i32& link_mid);

void d_height(u8 heights[6], i32& d_height, i32& d_height_sq);

void symm(u8 heights[6], i32& symm, i32& symm_sq);

void shape_u(u8 heights[6], i32& shape_u, i32& shape_u_sq);

constexpr Weight DEFAULT_WEIGHT = {
    .link_v = 300,
    .link_h = 300,
    .link_mid = -800,
    .d_height = -50,
    .d_height_sq = -50,
    .symm = -45,
    .symm_sq = -5,
    .shape_u = -200,
    .shape_u_sq = -150,

    .frame = -50,

    .ptnl_chain_count = 2000,
    .ptnl_chain_score = 10,
    .ptnl_chain_needed = -500,
    .ptnl_chain_height = 1000,
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    link_v,
    link_h,
    link_mid,
    d_height,
    d_height_sq,
    symm,
    symm_sq,
    shape_u,
    shape_u_sq,
    frame,
    ptnl_chain_count,
    ptnl_chain_score,
    ptnl_chain_needed,
    ptnl_chain_height
)

};