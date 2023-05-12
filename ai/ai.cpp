#include "ai.h"

namespace AI
{

Result think_1p(Field field, std::vector<Cell::Pair> queue, Eval::Weight w, i32 trigger_score)
{
    auto search_result = Search::search(field, queue, w);

    if (search_result.candidates.empty()) {
        return Result {
            .placement = { .x = 2, .r = Direction::Type::UP },
            .eval = -1
        };
    }

    return build(search_result, field, trigger_score);
};

Result think_2p(Field field, std::vector<Cell::Pair> queue, Data data, Enemy enemy, Eval::Weight w)
{
    auto search_attacks = Search::search_attacks(field, queue, false);

    // auto enemy_danger = AI::get_enemy_danger(data, enemy, field);

    // If enemy attacked
    if (enemy.attack > 0) {
        // Find all attacks in possible time frame
        std::vector<std::pair<u32, Search::Attack>> candidate_attack;
        std::vector<std::pair<u32, Search::Attack>> candidate_attack_all;
        i32 best_attack = 0;

        for (u32 i = 0; i < search_attacks.candidates.size(); ++i) {
            if (search_attacks.candidates[i].attacks.empty()) {
                continue;
            }

            for (auto& attack : search_attacks.candidates[i].attacks) {
                if (attack.frame > enemy.attack_frame) {
                    continue;
                }

                // These are attacks that can offset the enemy's attack
                if ((attack.score + data.bonus) / data.target + data.all_clear * 30 >= enemy.attack) {
                    candidate_attack.push_back({i, attack});
                }

                // These are all attacks in the possible time frame, no matter if they can offset the enemy's attack or not
                candidate_attack_all.push_back({i, attack});

                best_attack = std::max(best_attack, attack.score + data.bonus + data.all_clear * data.target * 30);
            }
        }

        // If there aren't any possible offset attacks in the given time frame
        if (candidate_attack.empty()) {
            // And if there are still some attacks (but can't offset) and the remaining time is small
            if ((enemy.attack_frame <= 4) && (!candidate_attack_all.empty())) {
                // Then trigger the biggest possible chain
                auto best_candidate = *std::max_element(
                    candidate_attack_all.begin(),
                    candidate_attack_all.end(),
                    [&] (const std::pair<u32, Search::Attack>& a, const std::pair<u32, Search::Attack>& b) {
                        if (enemy.all_clear && enemy.attack <= 24) {
                            if (a.second.all_clear == b.second.all_clear) {
                               return a.second.score < b.second.score;
                            }
                            return a.second.all_clear < b.second.all_clear;
                        }
                        return a.second.score < b.second.score;
                    }
                );

                if (best_candidate.second.score + data.bonus + data.all_clear * data.target * 30 >= enemy.attack * data.target - 30 * data.target) {
                    return Result {
                        .placement = search_attacks.candidates[best_candidate.first].placement,
                        .eval = -1
                    };
                }
            }
            // If there aren't any possible offset attacks but the remaining time is large
            else {
                // Then try to build chain fast
                return think_1p(field, queue, Eval::FAST_WEIGHT);
            }
        }
        // Else, if there are possible offset attacks
        else {
            auto best_candidate = std::pair<u32, Search::Attack>();
            
            // Return the enemy's attack with the biggest chain if:
            //  - The enemy's attack is too big
            //  - Our main chain is big enough
            //  - Our field is big enough
            //  - The enemy just triggered their biggest chain
            //  - The enemy is in danger
            if (enemy.attack >= 2240 ||
                best_attack >= 70000 ||
                field.get_count() >= 60 ||
                AI::get_small_field(enemy.field, field) ||
                AI::get_garbage_obstruct(enemy.field)) {
                best_candidate = *std::max_element(
                    candidate_attack.begin(),
                    candidate_attack.end(),
                    [&] (const std::pair<u32, Search::Attack>& a, const std::pair<u32, Search::Attack>& b) {
                        return a.second.score < b.second.score;
                    }
                );
            }
            // Else, if the enemy's attack is just a small harassment
            else {
                // Then try to return it with a fast chain with big attacks (power chain)
                best_candidate = *std::max_element(
                    candidate_attack.begin(),
                    candidate_attack.end(),
                    [&] (const std::pair<u32, Search::Attack>& a, const std::pair<u32, Search::Attack>& b) {
                        bool a_over_enemy = (a.second.score + data.bonus) / data.target + data.all_clear * 30 > enemy.attack;
                        bool b_over_enemy = (b.second.score + data.bonus) / data.target + data.all_clear * 30 > enemy.attack;

                        if (a_over_enemy != b_over_enemy) {
                            return a_over_enemy < b_over_enemy;
                        }

                        // if (a.second.count != b.second.count) {
                        //     return a.second.count > b.second.count;
                        // }

                        // if (a.second.all_clear != b.second.all_clear) {
                        //     return a.second.all_clear < b.second.all_clear;
                        // }

                        // if (a.second.frame != b.second.frame) {
                        //     return a.second.frame > b.second.frame;
                        // }

                        if (a.second.count * 2 + a.second.frame != b.second.count * 2 + b.second.frame) {
                            return a.second.count * 2 + a.second.frame > b.second.count * 2 + b.second.frame;
                        }

                        if (a.second.all_clear != b.second.all_clear) {
                            return a.second.all_clear < b.second.all_clear;
                        }

                        return a.second.score < b.second.score;
                    }
                );
            }

            return Result {
                .placement = search_attacks.candidates[best_candidate.first].placement,
                .eval = search_attacks.candidates[best_candidate.first].eval
            };
        }
    }

    // If the enemy is being obstructed by garbage
    if (AI::get_garbage_obstruct(enemy.field)) {
        // If we are having all clear, then trigger a small chain
        if (data.all_clear) {
            auto best_candidate = *std::max_element(
                search_attacks.candidates.begin(),
                search_attacks.candidates.end(),
                [&] (const Search::Candidate& a, const Search::Candidate& b) {
                    if (a.attacks[0].count != b.attacks[0].count) {
                        return a.attacks[0].count > b.attacks[0].count;
                    }
                    return a.attacks[0].score < b.attacks[0].score;
                }
            );

            if (best_candidate.attacks[0].count <= 4) {
                return Result {
                    .placement = best_candidate.placement,
                    .eval = -1
                };
            }
        }

        // Build fast and trigger chain fast
        return think_1p(field, queue, Eval::FAST_WEIGHT);
    }

    // If the enemy has all clear
    if (enemy.all_clear) {
        // Then build chain fast
        return think_1p(field, queue, Eval::FAST_WEIGHT);
    }

    // Else, our enemy is not sending any attacks and they are in a danger position, then harass them
    if (AI::get_harassable(data, enemy, field) || AI::get_small_field(enemy.field, field)) {
        // Get all possible attacks now
        std::vector<Search::Candidate> attack_candidate;
        std::vector<Search::Candidate> attack_candidate_fast;

        for (auto& candidate : search_attacks.candidates) {
            if (candidate.attacks.empty()) {
                continue;
            }

            // These are candidates with attacks
            attack_candidate.push_back(candidate);

            // These are candidates that can trigger chain right away
            if (candidate.attacks[0].frame == 0) {
                attack_candidate_fast.push_back(candidate);
            }
        }

        if (!attack_candidate_fast.empty()) {
            auto best_candidate = *std::max_element(
                attack_candidate_fast.begin(),
                attack_candidate_fast.end(),
                [&] (const Search::Candidate& a, const Search::Candidate& b) {
                    if (a.attacks[0].count != b.attacks[0].count) {
                        return a.attacks[0].count > b.attacks[0].count;
                    }
                    return a.attacks[0].score < b.attacks[0].score;
                }
            );

            if (best_candidate.attacks[0].count <= 4 && best_candidate.attacks[0].score + data.bonus + data.all_clear * data.target * 30 >= 360) {
                return Result {
                    .placement = best_candidate.placement,
                    .eval = -1
                };
            }
        }
    }

    // Else, build chain normally
    return think_1p(field, queue, w);
};

Result build(Search::Result& search_result, Field& field, i32 trigger_score)
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
            if (attack.frame > 4 || attack.count > 4) {
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
                if (a.second.score == b.second.score) {
                    return a.second.frame < b.second.frame;
                }
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
    bool trigger = chain_score_max >= trigger_score;

    // Build chain
    if (!trigger) {
        auto best = *std::max_element(
            search_result.candidates.begin(),
            search_result.candidates.end(),
            [&] (const Search::Candidate& a, const Search::Candidate& b) {
                if (a.eval != b.eval) {
                    return a.eval < b.eval;
                }

                return a.attacks.size() < b.attacks.size();
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
        .eval = -2
    };
};

i32 get_unburied_count(Field& field)
{
    auto enemy_mask = field.get_mask();
    auto emeny_mask_empty = ~enemy_mask;
    auto enemy_mask_color = enemy_mask & (~field.data[static_cast<i32>(Cell::Type::GARBAGE)]);
    return (emeny_mask_empty | enemy_mask_color).get_mask_group(2, 11).get_count() - emeny_mask_empty.get_count();
};

i32 get_attack(Field& field, std::vector<Cell::Pair>& queue)
{
    auto search_result = Search::search_attacks(field, queue);

    if (search_result.candidates.empty()) {
        return 0;
    }

    i32 best = 0;

    for (auto& c : search_result.candidates) {
        for (auto& attack : c.attacks) {
            best = std::max(best, attack.score);
        }
    }

    return best;
};

bool get_garbage_obstruct(Field& field)
{
    i32 unburied_count = AI::get_unburied_count(field);
    i32 garbage_count = field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count();
    // i32 attack = AI::get_attack(field, )

    if (garbage_count < 1) {
        return false;
    }

    return
        (garbage_count >= (field.get_count() / 2)) ||
        (unburied_count <= field.get_count() / 2);
};

bool get_small_field(Field& field, Field& other)
{
    return other.get_count() > field.get_count() * 2;
};

bool get_harassable(Data& data, Enemy& enemy, Field& field)
{
    i32 enemy_attack = AI::get_attack(enemy.field, enemy.queue) + enemy.all_clear * 30 * data.target;

    return
        (enemy_attack <= 360 && field.get_count() >= 48 && field.get_count() <= 62) ||
        (enemy_attack <= 360 && (field.get_count() >= enemy.field.get_count() * 2)) ||
        (enemy_attack <= 360 && enemy.field.get_height(2) >= 11);
};

i32 get_enemy_danger(Data& data, Enemy& enemy, Field& field)
{
    i32 enemy_unburied_count = AI::get_unburied_count(enemy.field);
    i32 enemy_garbage_count = enemy.field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count();
    i32 enemy_attack = AI::get_attack(enemy.field, enemy.queue) + enemy.all_clear * 30 * data.target;

    // if (enemy_garbage_count > 0 && (
    //     (enemy_garbage_count >= (enemy.field.get_count() / 2)) ||
    //     (enemy_unburied_count <= enemy_garbage_count) ||
    //     (enemy_unburied_count <= 24 && enemy.field.get_count() >= 54))) {
    //     return DANGER_HIGH;
    // }

    // if ((field.get_count() >= enemy.field.get_count() * 2) ||
    //     (enemy_unburied_count <= enemy.field.get_count() / 2) ||
    //     (enemy_attack <= 360 && enemy.field.get_height(2) >= 11) ||
    //     (enemy_attack <= 360 && enemy.field.get_count() >= 62)) {
    //     return DANGER_MEDIUM;
    // }

    // if ((field.get_count() >= 48 && enemy_attack <= 360)) {
    //     return DANGER_LOW;
    // }

    // return DANGER_NONE;

    return
        (enemy_attack <= 360 && field.get_count() >= 48 && field.get_count() <= 62) ||
        (enemy_attack <= 360 && (field.get_count() > enemy.field.get_count() * 2)) ||
        (enemy_garbage_count >= (enemy.field.get_count() / 2)) ||
        (enemy_unburied_count <= enemy.field.get_count() / 2) ||
        (enemy_attack <= 360 && enemy.field.get_height(2) >= 11);
};

};