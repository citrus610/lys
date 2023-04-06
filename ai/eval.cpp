#include "eval.h"

namespace Eval
{

i32 evaluate(Field& field, std::optional<Detect::Score> detect, u8 frame, Weight& w)
{
    i32 result = 0;

    u8 heights[6];
    field.get_heights(heights);

    i32 link_v = 0;
    i32 link_h = 0;
    i32 link_mid = 0;
    Eval::link(field, link_v, link_h, link_mid);
    result += link_v * w.link_v;
    result += link_h * w.link_h;
    result += link_mid * w.link_mid;

    i32 d_height = 0;
    i32 d_height_sq = 0;
    Eval::d_height(heights, d_height, d_height_sq);
    result += d_height * w.d_height;
    result += d_height_sq * w.d_height_sq;

    i32 symm = 0;
    i32 symm_sq = 0;
    Eval::symm(heights, symm, symm_sq);
    result += symm * w.symm;
    result += symm_sq * w.symm_sq;

    i32 shape_u = 0;
    i32 shape_u_sq = 0;
    Eval::shape_u(heights, shape_u, shape_u_sq);
    result += shape_u * w.shape_u;
    result += shape_u_sq * w.shape_u_sq;

    result += frame * w.frame;

    // const auto pdata = Pattern::DEFAULT();
    // result += Pattern::evaluate(field, heights, pdata) * w.form;

    if (detect.has_value()) {
        result += (detect->chain.score >> 8) * w.ptnl_chain_score;
        result += detect->chain.count * w.ptnl_chain_count;
        result += detect->needed * w.ptnl_chain_needed;
        // if (*std::min_element(heights, heights + 6) > 3) {
            result += detect->height * w.ptnl_chain_height;
        // }
    }

    return result;
};

void link(Field& field, i32& link_v, i32& link_h, i32& link_mid)
{
    link_v = 0;
    link_h = 0;
    link_mid = 0;

    for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
        FieldBit m12 = field.data[p].get_mask_12();

        FieldBit v;
        v.data = _mm_slli_epi16(m12.data, 1) & m12.data;
        // v.data = _mm_slli_epi16(m12.data, 1) & _mm_set1_epi16(0b1111);
        link_v += v.get_count();

        FieldBit h;
        h.data = _mm_slli_si128(m12.data, 2) & m12.data;
        // h.data = _mm_slli_si128(m12.data, 2) & _mm_set1_epi16(0b1111);
        link_h += h.get_count();

        link_mid += std::popcount(u32(m12.get_col(2) & m12.get_col(3)));
        // v.data = v.data & _mm_set_epi16(0, 0, 0, 0xFFF, 0, 0, 0xFFF, 0);
        // link_mid += v.get_count();
    }
};

void d_height(u8 heights[6], i32& d_height, i32& d_height_sq)
{
    d_height = *std::max_element(heights, heights + 6) - *std::min_element(heights, heights + 6);
    d_height_sq = d_height * d_height;
};

void symm(u8 heights[6], i32& symm, i32& symm_sq)
{
    i32 value[3] = {
        std::abs(heights[0] - heights[5]),
        std::abs(heights[1] - heights[4]),
        std::abs(heights[2] - heights[3])
    };

    symm = value[0] + value[1] + value[2];
    symm_sq = value[0] * value[0] + value[1] * value[1] + value[2] * value[2];
};

void shape_u(u8 heights[6], i32& shape_u, i32& shape_u_sq)
{
    shape_u = 0;
    shape_u_sq = 0;

    for (i32 i = 0; i < 2; ++i) {
        if (heights[i] < heights[i + 1]) {
            i32 value = heights[i + 1] - heights[i];
            shape_u += value;
            shape_u_sq += value * value;
        }
    }

    for (i32 i = 3; i < 5; ++i) {
        if (heights[i] > heights[i + 1]) {
            i32 value = heights[i] - heights[i + 1];
            shape_u += value;
            shape_u_sq += value * value;
        }
    }
};

};