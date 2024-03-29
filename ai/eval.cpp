#include "eval.h"

namespace Eval
{

i32 evaluate(Field& field, std::optional<Detect::Result> detect, u8 frame, Weight& w)
{
    i32 result = 0;

    u8 heights[6];
    field.get_heights(heights);

    i32 height_ave = std::accumulate(heights, heights + 6, 0) / 6;

    i32 shape = 0;
    i32 shape_s = 0;
    // for (i32 i = 0; i < 2; ++i) {
    //     if (heights[i] < heights[i + 1]) {
    //         i32 value = heights[i + 1] - heights[i];
    //         shape += value;
    //         shape_s += value * value;
    //     }
    // }
    // for (i32 i = 3; i < 5; ++i) {
    //     if (heights[i] > heights[i + 1]) {
    //         i32 value = heights[i] - heights[i + 1];
    //         shape += value;
    //         shape_s += value * value;
    //     }
    // }

    i32 height_lowest = std::max(heights[2], heights[3]);

    const i32 coef[6] = { 4, 2, 0, 0, 2, 4 };

    for (i32 i = 0; i < 6; ++i) {
        i32 value = std::abs(heights[i] - height_lowest - coef[i]);
        shape += value;
        shape_s += value * value;
    }

    result += shape * w.shape;
    result += shape_s * w.shape_s;

    result += field.data[static_cast<u8>(Cell::Type::GARBAGE)].get_count() * w.nuisance;

    result += std::max(0, heights[0] + heights[1] + heights[2] - heights[3] - heights[4] - heights[5]) * w.side_bias;

    i32 disconnect = 0;
    i32 link_hor = 0;
    i32 link_ver = 0;
    for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
        FieldBit m12 = field.data[p].get_mask_12();

        FieldBit no;
        no.data =
            _mm_slli_si128(~m12.data, 2) &
            _mm_srli_si128(~m12.data, 2) &
            _mm_slli_epi16(~m12.data, 1) &
            _mm_srli_epi16(~m12.data, 1) &
            m12.data;
        disconnect += no.get_count();

        FieldBit hor;
        hor.data = _mm_slli_si128(m12.data, 2) & m12.data;
        link_hor += hor.get_count();

        FieldBit ver;
        ver.data = _mm_slli_epi16(m12.data, 1) & m12.data;
        link_ver += ver.get_count();
    }
    result += disconnect * w.disconnect;
    result += link_hor * w.link_hor;
    result += link_ver * w.link_ver;

    if (w.form != 0) {
        result += Form::evaluate(field, heights, Form::DEFAULT()) * w.form;
    }

    result += frame * w.frame;

    if (detect.has_value()) {
        result += (detect->main.chain.score >> 8) * w.chain_score;
        // result += (detect->main.height - height_ave) * w.chain_height;
        result += detect->main.chain.count * w.chain_count;
        result += detect->main.height * w.chain_height;
        result += detect->main.needed * w.chain_needed;
    }

    return result;
};

};