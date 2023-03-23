#include "ai.h"

namespace AI
{

Result think_1p(Field field, std::vector<Cell::Pair> queue, Eval::Weight w)
{
    auto search_result = Search::search(field, queue, w);

    if (search_result.candidates.empty()) {
        return Result {
            .placement = { .x = 2, .r = Direction::Type::UP },
            .eval = -1000000
        };
    }

    return build(search_result, field.get_count());
};

Result think_2p(Field field, std::vector<Cell::Pair> queue, Data data, Enemy enemy, Eval::Weight w)
{
    auto search_result = Search::search(field, queue, w);

    if (search_result.candidates.empty()) {
        return Result {
            .placement = { .x = 2, .r = Direction::Type::UP },
            .eval = -1000000
        };
    }

    // Offset attack
    if (enemy.attack > 0) {
        std::vector<std::pair<u32, Search::Attack>> candidate_attack;
        std::vector<std::pair<u32, Search::Attack>> candidate_attack_all;

        for (u32 i = 0; i < search_result.candidates.size(); ++i) {
            if (search_result.candidates[i].attacks.empty()) {
                continue;
            }

            for (auto attack : search_result.candidates[i].attacks) {
                if (attack.frame > enemy.attack_frame) {
                    continue;
                }

                if (attack.score + data.bonus + data.all_clear * data.target * 30 >= enemy.attack * data.target) {
                    candidate_attack.push_back({i, attack});
                }

                candidate_attack_all.push_back({i, attack});
            }
        }

        if (candidate_attack.empty()) {
            if ((enemy.attack_frame <= 4) && (!candidate_attack_all.empty())) {
                auto best_candidate = *std::max_element(
                    candidate_attack_all.begin(),
                    candidate_attack_all.end(),
                    [&] (const std::pair<u32, Search::Attack>& a, const std::pair<u32, Search::Attack>& b) {
                        return a.second.score < b.second.score;
                    }
                );

                if (best_candidate.second.score + data.bonus + data.all_clear * data.target * 30 >= enemy.attack * data.target - 30 * data.target) {
                    return Result {
                        .placement = search_result.candidates[best_candidate.first].placement,
                        .eval = search_result.candidates[best_candidate.first].eval
                    };
                }
            }
        }
        else {
            auto best_candidate = *std::max_element(
                candidate_attack.begin(),
                candidate_attack.end(),
                [&] (const std::pair<u32, Search::Attack>& a, const std::pair<u32, Search::Attack>& b) {
                    if (a.second.count == b.second.count) {
                        if (a.second.all_clear == b.second.all_clear) {
                            if (a.second.frame == b.second.frame) {
                                return a.second.score < b.second.score;
                            }

                            return a.second.frame > b.second.frame;
                        }

                        return a.second.all_clear < b.second.all_clear;
                    }

                    return a.second.count > b.second.count;
                }
            );

            return Result {
                .placement = search_result.candidates[best_candidate.first].placement,
                .eval = search_result.candidates[best_candidate.first].eval
            };
        }
    }

    // Attack if enemy is in danger
    else if (AI::get_enemy_danger(data, enemy, field)) {
        std::vector<Search::Candidate> attack_candidate;

        for (auto& candidate : search_result.candidates) {
            if (!candidate.attacks.empty() && candidate.attacks[0].frame == 0 && candidate.attacks[0].count <= 5 && candidate.attacks[0].score + data.bonus + data.all_clear * data.target * 30 >= 9 * data.target) {
                attack_candidate.push_back(candidate);
            }
        }

        if (!attack_candidate.empty()) {
            std::sort(
                attack_candidate.begin(),
                attack_candidate.end(),
                [&] (const Search::Candidate& a, const Search::Candidate& b) {
                    if (a.attacks[0].count == b.attacks[0].count) {
                        return a.attacks[0].score > b.attacks[0].score;
                    }
                    return a.attacks[0].count < b.attacks[0].count;
                }  
            );

            return Result {
                .placement = attack_candidate[0].placement,
                .eval = INT32_MIN,
            };
        }
    }

    return build(search_result, field.get_count());
};

Result build(Search::Result& search_result, u32 field_count)
{
    // All clear
#ifdef TUNER
#else
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
#endif

    // Check trigger chain condition
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

    // Build chain
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

    // Trigger chain
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

    // Error
    return Result {
        .placement = { .x = 2, .r = Direction::Type::UP },
        .eval = -1000001
    };
};

bool get_enemy_danger(Data& data, Enemy& enemy, Field& field)
{
    auto enemy_detect = Detect::detect(enemy.field);
    auto enemy_attack = std::max(enemy_detect.flat, enemy_detect.deep);

    bool enemy_no_attack = (enemy_attack.chain.count <= 1) || (enemy_attack.chain.score += enemy.all_clear * 30 * data.target <= 720);

    bool enemy_high = enemy.field.get_height(2) > 10;

    bool enemy_many_garbage = enemy.field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count() >= (enemy.field.get_count() / 2);

    auto enemy_mask = enemy.field.get_mask();
    auto emeny_mask_empty = ~enemy_mask;
    auto enemy_mask_color = enemy_mask & (~enemy.field.data[static_cast<i32>(Cell::Type::GARBAGE)]);
    i32 enemy_unobstructed_count = (emeny_mask_empty | enemy_mask_color).get_mask_group(2, 11).get_count() - emeny_mask_empty.get_count();
    bool enemy_obstructed = enemy_unobstructed_count <= (enemy.field.get_count() / 2);

    return
        (enemy_no_attack && enemy.field.get_count() >= 42 && field.get_count() >= 42) ||
        (enemy_no_attack && (field.get_count() >= enemy.field.get_count() * 2)) ||
        enemy_many_garbage ||
        enemy_obstructed ||
        (enemy_high && enemy_attack.chain.score <= 720);
};

};