#include "eval.h"

namespace Eval
{

i32 evaluate(Field& field, std::optional<Detect::Result> detect, u8 frame, Weight& w)
{
    i32 result = 0;

    u8 heights[6];
    field.get_heights(heights);

    i32 link_2 = 0;
    i32 link_3 = 0;
    i32 link_mid = 0;
    Eval::link(field, link_2, link_3, link_mid);
    result += link_2 * w.link_2;
    result += link_3 * w.link_3;
    result += link_mid * w.link_mid;

    i32 bump = 0;
    i32 bump_sq = 0;
    Eval::bump(heights, bump, bump_sq);
    result += bump * w.bump;
    result += bump_sq * w.bump_sq;

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

    if (detect.has_value()) {
        result += (detect->main.chain.score >> 8) * w.ptnl_chain_score;
        result += detect->main.chain.count * w.ptnl_chain_count;
        result += detect->main.needed * w.ptnl_chain_needed;
        result += detect->main.height * w.ptnl_chain_height;

        result += (detect->harass.chain.score >> 8) * w.harass_score;
        result += detect->harass.chain.count * w.harass_count;
        result += detect->harass.needed * w.harass_needed;
        result += detect->harass.height * w.harass_height;
    }

    return result;
};

void link(Field& field, i32& link_2, i32& link_3, i32& link_mid)
{
    for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
        FieldBit m12 = field.data[p].get_mask_12();

        __m128i r = _mm_srli_si128(m12.data, 2) & m12.data;
        __m128i l = _mm_slli_si128(m12.data, 2) & m12.data;
        __m128i u = _mm_srli_epi16(m12.data, 1) & m12.data;
        __m128i d = _mm_slli_epi16(m12.data, 1) & m12.data;

        __m128i ud_and = u & d;
        __m128i lr_and = l & r;
        __m128i ud_or = u | d;
        __m128i lr_or = l | r;

        FieldBit m2, m3;
        m3.data = (ud_or & lr_or) | ud_and | lr_and;
        m2.data = _mm_andnot_si128(m3.get_expand().data & m12.data, l | u);

        link_2 += m2.get_count();
        link_3 += m3.get_count();
        link_mid += std::popcount(u32(m12.get_col(2) & m12.get_col(3)));
    }
};

void bump(u8 heights[6], i32& bump, i32& bump_sq)
{
    // for (i32 i = 0; i < 5; ++i) {
    //     bump += std::abs(heights[i] - heights[i + 1]);
    //     bump_sq += (heights[i] - heights[i + 1]) * (heights[i] - heights[i + 1]);
    // }
    bump = *std::max_element(heights, heights + 6) - *std::min_element(heights, heights + 6);
    bump_sq = bump * bump;
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