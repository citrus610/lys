#include "ai.h"

namespace AI
{

Result think_1p(Field field, std::vector<Cell::Pair> queue, Eval::Weight w)
{
    auto result_a = Search::Attack::search(field, queue);
    auto result_b = Search::Beam::search(field, queue, w);

    if (result_a.candidates.empty()) {
        return Result {
            .placement = { .x = 2, .r = Direction::Type::UP },
            .eval = -1000000
        };
    }

    return build(result_a, result_b, field.get_count());
};

Result build(Search::Attack::Result& result_a, Search::Beam::Result& result_b, u32 field_count)
{
    // All clear
#ifdef TUNER
#else
    std::vector<std::pair<u32, Search::Attack::Attack>> all_clear_attacks;
    for (u32 i = 0; i < result_a.candidates.size(); ++i) {
        if (result_a.candidates[i].attacks.empty()) {
            continue;
        }
        for (auto attack : result_a.candidates[i].attacks) {
            if (attack.frame > 6 || attack.count > 3) {
                continue;
            }
            if (attack.all_clear) {
                all_clear_attacks.push_back({i, attack});
            }
        }
    }
    if (!all_clear_attacks.empty()) {
        std::sort(
            all_clear_attacks.begin(),
            all_clear_attacks.end(),
            [&] (const std::pair<u32, Search::Attack::Attack>& a, const std::pair<u32, Search::Attack::Attack>& b) {
                return a.second.score > b.second.score;
            }
        );
        return Result {
            .placement = result_a.candidates[all_clear_attacks[0].first].placement,
            .eval = 0,
        };
    }
#endif

    // Check trigger chain condition
    i32 chain_score_max = 0;
    i32 attack_count = 0;
    for (auto candidate : result_a.candidates) {
        for (auto attack : candidate.attacks) {
            chain_score_max = std::max(chain_score_max, attack.score);
        }
        attack_count += candidate.attacks.size();
    }
    bool trigger = chain_score_max >= 70000;

    // Build chain
    if (!trigger) {
        if (result_b.eval > INT32_MIN) {
            return Result {
                .placement = result_b.placement,
                .eval = result_b.eval,
            };
        }
    }

    // Trigger chain
    if (attack_count > 0) {
        std::pair<i32, Result> best = { 0, Result() };

        for (auto& c : result_a.candidates) {
            if (c.attacks.empty()) {
                continue;
            }

            Search::Attack::Attack attack;

            attack = *std::max_element(
                c.attacks.begin(),
                c.attacks.end(),
                [&] (const Search::Attack::Attack& a, const Search::Attack::Attack& b) {
                    return a.score < b.score;
                }
            );

            if (best.first < attack.score) {
                best.first = attack.score;
                best.second.placement = c.placement;
                best.second.eval = attack.score;
            }
        }

        return best.second;
    }

    // Error
    return Result {
        .placement = { .x = 2, .r = Direction::Type::UP },
        .eval = -1
    };
};

bool get_enemy_danger(Data& data, Enemy& enemy, Field& field)
{
    auto enemy_detect = Detect::detect(enemy.field);
    auto enemy_attack = enemy_detect.main;

    bool enemy_no_attack = (enemy_attack.chain.count <= 1) || (enemy_attack.chain.score += enemy.all_clear * 30 * data.target <= 720);

    bool enemy_high = enemy.field.get_height(2) > 10;

    bool enemy_many_garbage = enemy.field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count() >= (enemy.field.get_count() / 2);

    auto enemy_mask = enemy.field.get_mask();
    auto emeny_mask_empty = ~enemy_mask;
    auto enemy_mask_color = enemy_mask & (~enemy.field.data[static_cast<i32>(Cell::Type::GARBAGE)]);
    i32 enemy_unobstructed_count = (emeny_mask_empty | enemy_mask_color).get_mask_group(2, 11).get_count() - emeny_mask_empty.get_count();
    bool enemy_obstructed = enemy_unobstructed_count <= (enemy.field.get_count() / 2);

    return
        (enemy_no_attack && enemy.field.get_count() >= 48 && field.get_count() >= 48) ||
        (enemy_no_attack && (field.get_count() >= enemy.field.get_count() * 2)) ||
        enemy_many_garbage ||
        enemy_obstructed ||
        (enemy_high && enemy_attack.chain.score <= 720);
};

};