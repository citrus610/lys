#include "detect.h"

namespace Detect
{

Result detect(Field& field)
{
    Result result;

    u8 heights[6];
    field.get_heights(heights);

    i8 min_x = 0;
    for (i8 i = 2; i >= 0; --i) {
        if (heights[i] > 11) {
            min_x = i + 1;
            break;
        }
    }

    for (i8 x = min_x; x < 6; ++x) {
        if (heights[x] > 11) {
            break;
        }

        u8 max_puyo_add = std::min(
            (Detect::is_well(heights, x) ? 1 : 3),
            12 - heights[x] - (x == 2)
        );

        for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
            Field copy = field;

            u8 i = 0;
            for (i = 0; i < max_puyo_add; ++i) {
                copy.data[p].set_bit(x, heights[x] + i);

                if (copy.data[p].get_mask_group_4(x, heights[x]).get_count() >= 4) {
                    break;
                }
            }

            auto chain_mask = copy.pop();
            auto chain_score = Chain::get_score(chain_mask);

            result.main = std::max(
                result.main,
                Score {
                    .chain = chain_score,
                    .needed = i + 1,
                    .height = heights[x]
                },
                Detect::cmp_main
            );

            // if (chain_score.score >= 100 && chain_score.score <= 5000) {
            //     result.harass = std::max(
            //         result.harass,
            //         Score {
            //             .chain = chain_score,
            //             .needed = i + 1,
            //             .height = heights[x]
            //         },
            //         Detect::cmp_sub
            //     );
            // }

            if (chain_score.count == 1 && heights[x] > 1 && heights[x] + i < 10 && Detect::is_reachable(field, heights, x, i, p)) {
                for (u8 p_drop = 0; p_drop < Cell::COUNT - 1; ++p_drop) {
                    if (p_drop == p) {
                        continue;
                    }

                    auto copy_deep = copy;
                    copy_deep.drop_puyo(x, Cell::Type(p_drop));

                    auto chain_deep_mask = copy_deep.pop();
                    chain_deep_mask.insert(chain_mask[0], 0);

                    result.main = std::max(
                        result.main,
                        Score {
                            .chain = Chain::get_score(chain_deep_mask),
                            .needed = i + 2,
                            .height = heights[x]
                        },
                        Detect::cmp_main
                    );
                }
            }

            // if (chain_score.count == 1 && heights[x] > 1 && heights[x] + i < 10 && Detect::is_reachable(field, heights, x, i, p)) {
            //     u8 trigger_height = copy.get_height(x);
            //     copy.drop_puyo(x, copy.get_cell(x, trigger_height - 1));

            //     auto previous_link = chain_mask[0];
            //     chain_mask = copy.pop();
            //     chain_mask.insert(previous_link, 0);

            //     result.main = std::max(
            //         result.main,
            //         Score {
            //             .chain = Chain::get_score(chain_mask),
            //             .needed = i + 2,
            //             .height = heights[x]
            //         },
            //         Detect::cmp_main
            //     );
            // }
        }
    }

    return result;
};

Result detect_fast(Field& field)
{
    Result result;

    u8 heights[6];
    field.get_heights(heights);

    i8 min_x = 0;
    for (i8 i = 2; i >= 0; --i) {
        if (heights[i] > 11) {
            min_x = i + 1;
            break;
        }
    }

    for (i8 x = min_x; x < 6; ++x) {
        if (heights[x] > 11) {
            break;
        }

        u8 max_puyo_add = std::min(1, 12 - heights[x] - (x == 2));

        for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
            Field copy = field;

            u8 i = 0;
            for (i = 0; i < max_puyo_add; ++i) {
                copy.data[p].set_bit(x, heights[x] + i);

                if (copy.data[p].get_mask_group_4(x, heights[x]).get_count() >= 4) {
                    break;
                }
            }

            auto chain_mask = copy.pop();
            auto chain_score = Chain::get_score(chain_mask);

            result.main = std::max(
                result.main,
                Score {
                    .chain = chain_score,
                    .needed = i + 1,
                    .height = heights[x]
                },
                Detect::cmp_main
            );

            // if (chain_score.score >= 100 && chain_score.score <= 5000) {
            //     result.harass = std::max(
            //         result.harass,
            //         Score {
            //             .chain = chain_score,
            //             .needed = i + 1,
            //             .height = heights[x]
            //         },
            //         Detect::cmp_sub
            //     );
            // }
        }
    }

    return result;
};

bool is_well(u8 heights[6], i8 x)
{
    if (x == 0) {
        return heights[0] < heights[1];
    }

    if (x == 5) {
        return heights[5] < heights[4];
    }

    return heights[x] < heights[x - 1] && heights[x] < heights[x + 1];
};

bool is_reachable(Field& field, u8 heights[6], i8 x, u8 added, u8 p)
{
    if (!field.data[p].get_bit(x, heights[x] - 1)) {
        return false;
    }

    bool well = false;
    if (x == 0) {
        if (field.data[p].get_bit(1, heights[0])) {
            return false;
        }

        well = heights[0] < heights[1] || (heights[0] == heights[1] && added < 1);
    }
    else if (x == 5) {
        if (field.data[p].get_bit(4, heights[5])) {
            return false;
        }

        well = heights[5] < heights[4] || (heights[5] == heights[4] && added < 1);
    }
    else {
        if (field.data[p].get_bit(x + 1, heights[x]) || field.data[p].get_bit(x - 1, heights[x])) {
            return false;
        }

        well =
            (heights[x] < heights[x - 1] && heights[x] < heights[x + 1]) ||
            ((heights[x] == heights[x - 1] || heights[x] == heights[x + 1]) && added < 1);
    }

    if (well) {
        return false;
    }

    return true;
};

};