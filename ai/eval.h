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
    i32 link_2 = 0;
    i32 link_3 = 0;
    i32 link_mid = 0;
    i32 bump = 0;
    i32 bump_sq = 0;
    i32 symm = 0;
    i32 symm_sq = 0;
    i32 shape_u = 0;
    i32 shape_u_sq = 0;

    i32 frame = 0;

    i32 ptnl_chain_count = 0;
    i32 ptnl_chain_score = 0;
    i32 ptnl_chain_needed = 0;
    i32 ptnl_chain_height = 0;

    i32 harass_count = 0;
    i32 harass_score = 0;
    i32 harass_needed = 0;
    i32 harass_height = 0;
};

i32 evaluate(Field& field, std::optional<Detect::Result> detect, u8 frame, Weight& w);

void link(Field& field, i32& link_2, i32& link_3, i32& link_mid);

void bump(u8 heights[6], i32& bump, i32& bump_sq);

void symm(u8 heights[6], i32& symm, i32& symm_sq);

void shape_u(u8 heights[6], i32& shape_u, i32& shape_u_sq);

constexpr Weight DEFAULT_WEIGHT = {
    .link_2 = 50,
    .link_3 = 300,
    .link_mid = -800,
    .bump = -50,
    .bump_sq = -50,
    .symm = 0,
    .symm_sq = -20,
    .shape_u = -200,
    .shape_u_sq = -150,

    .frame = -50,

    .ptnl_chain_count = 2000,
    .ptnl_chain_score = 10,
    .ptnl_chain_needed = -1000,
    .ptnl_chain_height = 800,

    .harass_count = 100,
    .harass_score = 100,
    .harass_needed = -500,
    .harass_height = 200

};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Weight,
    link_2,
    link_3,
    link_mid,
    bump,
    bump_sq,
    symm,
    symm_sq,
    shape_u,
    shape_u_sq,
    frame,
    ptnl_chain_count,
    ptnl_chain_score,
    ptnl_chain_needed,
    ptnl_chain_height,
    harass_count,
    harass_score,
    harass_needed,
    harass_height
)

};