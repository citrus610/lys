#include "detect.h"

namespace Detect
{

Score detect(Field& field)
{
    Score result;

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

                result = std::max(
                    result,
                    Score {
                        .chain = Chain::get_score(chain_mask),
                        .needed = i + 1,
                        .height = heights[x]
                    }
                );

                break;
            }
        }
    }

    return result;
};

};