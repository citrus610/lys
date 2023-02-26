#include "ai.h"

namespace AI
{

Result think_1p(Field field, std::vector<Cell::Pair> queue, Eval::Weight w)
{
    auto search_result = Search::search(field, queue, w);

    if (search_result.candidates.empty()) {
        return Result {
            .placement = { .x = 2, .r = Direction::Type::UP },
            .eval = INT32_MIN
        };
    }

    return build(search_result, field.get_count());
};

Result build(Search::Result& search_result, u32 field_count)
{
    std::vector<std::pair<u32, Search::Attack>> all_clear_attacks;
    for (u32 i = 0; i < search_result.candidates.size(); ++i) {
        if (search_result.candidates[i].attacks.empty()) {
            continue;
        }
        for (auto attack : search_result.candidates[i].attacks) {
            if (attack.frame > 6 || attack.count > 4) {
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
            [&] (const std::pair<u32, Search::Attack>& a, const std::pair<u32, Search::Attack>& b) {
                return a.second.score > b.second.score;
            }
        );
        return Result {
            .placement = search_result.candidates[all_clear_attacks[0].first].placement,
            .eval = search_result.candidates[all_clear_attacks[0].first].eval,
        };
    }

    i32 chain_score_max = 0;
    i32 attack_count = 0;
    for (auto candidate : search_result.candidates) {
        for (auto attack : candidate.attacks) {
            chain_score_max = std::max(chain_score_max, attack.score);
        }
        attack_count += candidate.attacks.size();
    }
    // bool trigger = (13 * 6 - field_count <= 16) || (chain_score_max >= 70000);
    bool trigger = chain_score_max >= 70000;

    if (!trigger) {
        auto best = *std::max_element(
            search_result.candidates.begin(),
            search_result.candidates.end(),
            [&] (const Search::Candidate& a, const Search::Candidate& b) {
                return a.eval < b.eval;
            }
        );
        
        if (best.eval > INT32_MIN) {
            return Result {
                .placement = best.placement,
                .eval = best.eval,
            };
        }
    }

    if (attack_count > 0) {
        std::pair<i32, Result> best = { 0, Result() };

        for (auto& c : search_result.candidates) {
            if (c.attacks.empty()) {
                continue;
            }

            auto attack = *std::max_element(
                c.attacks.begin(),
                c.attacks.end(),
                [&] (const Search::Attack& a, const Search::Attack& b) {
                    return a.score < b.score;
                }
            );

            if (best.first < attack.score) {
                best.first = attack.score;
                best.second.placement = c.placement;
                best.second.eval = c.eval;
            }
        }

        return best.second;
    }

    return Result {
        .placement = { .x = 2, .r = Direction::Type::UP },
        .eval = INT32_MIN
    };
};

};