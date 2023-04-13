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

        u8 max_puyo_add = std::min(3, 12 - heights[x] - (x == 2));

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

            if (chain_score.score >= 100 && chain_score.score <= 5000) {
                result.harass = std::max(
                    result.harass,
                    Score {
                        .chain = chain_score,
                        .needed = i + 1,
                        .height = heights[x]
                    },
                    Detect::cmp_sub
                );
            }
        }
    }

    return result;
};

Result detect_deep(Field& field)
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

        u8 max_puyo_add = std::min(3, 12 - heights[x] - (x == 2));

        for (u8 p = 0; p < Cell::COUNT - 1; ++p) {
            Field copy = field;

            u8 i = 0;
            for (i = 0; i < max_puyo_add; ++i) {
                copy.data[p].set_bit(x, heights[x] + i);

                if (copy.data[p].get_mask_group_4(x, heights[x]).get_count() >= 4) {
                    break;
                }
            }

            for (i8 x_key = min_x; x_key < 6; ++x_key) {
                if (heights[x_key] > 11) {
                    break;
                }

                if (x_key == x || heights[x_key] == 0) {
                    continue;
                }

                for (u8 p_key = 0; p_key < Cell::COUNT - 1; ++p_key) {
                    if (copy.data[p_key].get_bit(x_key, heights[x_key] - 1)) {
                        continue;
                    }

                    Field copy_key = copy;
                    copy_key.data[p_key].set_bit(x_key, heights[x_key]);

                    auto chain_mask = copy_key.pop();
                    auto chain_score = Chain::get_score(chain_mask);

                    result.main = std::max(
                        result.main,
                        Score {
                            .chain = chain_score,
                            .needed = i + 2,
                            .height = heights[x]
                        },
                        Detect::cmp_main
                    );
                }
            }

            auto chain_mask = copy.pop();
            auto chain_score = Chain::get_score(chain_mask);

            if (chain_score.score >= 100 && chain_score.score <= 5000) {
                result.harass = std::max(
                    result.harass,
                    Score {
                        .chain = chain_score,
                        .needed = i + 1,
                        .height = heights[x]
                    },
                    Detect::cmp_sub
                );
            }
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

};