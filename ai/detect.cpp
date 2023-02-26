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

            for (u8 i = 0; i < max_puyo_add; ++i) {
                copy.data[p].set_bit(x, heights[x] + i);

                if (copy.data[p].get_mask_group_4(x, heights[x]).get_count() < 4) {
                    continue;
                }

                auto chain_mask = copy.pop();

                result.flat = std::max(
                    result.flat,
                    Score {
                        .chain = Chain::get_score(chain_mask),
                        .needed = i + 1,
                        .height = heights[x]
                    }
                );

                if (chain_mask.get_size() == 1 && heights[x] + i < 10 && !(Detect::is_well(heights, x) && i >= 2)) {
                    u8 trigger_height = copy.get_height(x);
                    copy.drop_puyo(x, copy.get_cell(x, trigger_height - 1));

                    auto previous_link = chain_mask[0];
                    chain_mask = copy.pop();
                    chain_mask.insert(previous_link, 0);

                    result.deep = std::max(
                        result.deep,
                        Score {
                            .chain = Chain::get_score(chain_mask),
                            .needed = i + 2,
                            .height = trigger_height
                        }
                    );

                    break;
                }

                break;
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