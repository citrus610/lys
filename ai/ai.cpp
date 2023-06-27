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

    return AI::build(search_result, field, trigger_score);
};

Result think_2p(Field field, std::vector<Cell::Pair> queue, Data data, Enemy enemy, Eval::Weight w)
{
    auto search_attacks = Search::search_attacks(field, queue, false);

    // If enemy attacked
    if (enemy.attack > 0) {
        // If enemy attack is small and we are still in the beginning of the game, then we try to build tall and ignore it
        if (!data.all_clear &&
            enemy.attack <= 9 &&
            field.get_count() < 42 &&
            field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count() < 12) {
            // printf("receive small garbage!\n");
            return AI::think_1p(field, queue, Eval::FAST_WEIGHT);
        }

        // Find all attacks in possible time frame
        std::vector<std::pair<u32, Search::Attack>> candidate_attack;
        std::vector<std::pair<u32, Search::Attack>> candidate_attack_all;
        i32 best_attack = 0;
        i32 best_all_clear = 0;

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

                if (attack.all_clear) {
                    best_all_clear = std::max(best_all_clear, attack.score);
                }
            }
        }

        // If there aren't any possible offset attacks in the given time frame
        if (candidate_attack.empty()) {
            // If enemy is triggering an all clear chain and we also found an all clear chain, then trigger it
            if (enemy.all_clear &&
                best_all_clear > 0 &&
                best_all_clear + data.bonus + data.all_clear * data.target * 30 >= enemy.attack * data.target - 30 * data.target) {
                auto best_candidate = *std::max_element(
                    candidate_attack_all.begin(),
                    candidate_attack_all.end(),
                    [&] (const std::pair<u32, Search::Attack>& a, const std::pair<u32, Search::Attack>& b) {
                        if (a.second.all_clear != b.second.all_clear) {
                            return a.second.all_clear < b.second.all_clear;
                        }
                        return a.second.score < b.second.score;
                    }
                );

                // printf("return attack w all clear\n");

                return Result {
                    .placement = search_attacks.candidates[best_candidate.first].placement,
                    .eval = -1
                };
            }

            // And if there are still some attacks (but can't offset) and the remaining time is small
            if ((enemy.attack_frame <= 4) && (!candidate_attack_all.empty())) {
                // Then trigger the biggest possible chain
                auto best_candidate = *std::max_element(
                    candidate_attack_all.begin(),
                    candidate_attack_all.end(),
                    [&] (const std::pair<u32, Search::Attack>& a, const std::pair<u32, Search::Attack>& b) {
                        return a.second.score < b.second.score;
                    }
                );

                if (best_candidate.second.score + data.bonus + data.all_clear * data.target * 30 >= enemy.attack * data.target - 30 * data.target) {
                    // printf("return attack but not enough\n");

                    return Result {
                        .placement = search_attacks.candidates[best_candidate.first].placement,
                        .eval = -1
                    };
                }
            }
            // If there aren't any possible offset attacks but the remaining time is large
            else {
                // Then try to build chain fast
                return AI::think_1p(field, queue, Eval::FAST_WEIGHT);
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
                AI::get_garbage_obstruct(enemy.field, enemy.queue)) {
                best_candidate = *std::max_element(
                    candidate_attack.begin(),
                    candidate_attack.end(),
                    [&] (const std::pair<u32, Search::Attack>& a, const std::pair<u32, Search::Attack>& b) {
                        return a.second.score < b.second.score;
                    }
                );

                // printf("return attack biggest\n");
            }
            // Else, if the enemy's attack is just a small harassment
            else {
                // Then try to return it with a fast chain with big attacks (power chain)
                best_candidate = *std::max_element(
                    candidate_attack.begin(),
                    candidate_attack.end(),
                    [&] (const std::pair<u32, Search::Attack>& a, const std::pair<u32, Search::Attack>& b) {
                        bool a_main_chain = a.second.score > 8680 || a.second.count > 6;
                        bool b_main_chain = b.second.score > 8680 || b.second.count > 6;

                        if (a_main_chain && b_main_chain) {
                            return a.second.score < b.second.score;
                        }

                        if (a_main_chain && !b_main_chain) {
                            return true;
                        }

                        if (!a_main_chain && b_main_chain) {
                            return false;
                        }

                        i32 a_over_enemy = (a.second.score + data.bonus) / data.target + data.all_clear * 30 > enemy.attack;
                        i32 b_over_enemy = (b.second.score + data.bonus) / data.target + data.all_clear * 30 > enemy.attack;

                        if (a_over_enemy != b_over_enemy) {
                            return a_over_enemy < b_over_enemy;
                        }

                        if (a.second.count != b.second.count) {
                            return a.second.count > b.second.count;
                        }

                        if (a.second.all_clear != b.second.all_clear) {
                            return a.second.all_clear < b.second.all_clear;
                        }

                        if (a.second.frame != b.second.frame) {
                            return a.second.frame > b.second.frame;
                        }

                        return a.second.score < b.second.score;
                    }
                );

                // printf("return attack small\n");
            }

            return Result {
                .placement = search_attacks.candidates[best_candidate.first].placement,
                .eval = search_attacks.candidates[best_candidate.first].eval
            };
        }
    }

    // If we are in danger or the enemy is having all clear, them build fast and safely
    if (enemy.all_clear) {
        return AI::think_1p(field, queue, Eval::FAST_WEIGHT);
    }

    // If the enemy is being obstructed by garbage
    if (AI::get_garbage_obstruct(enemy.field, enemy.queue)) {
        // printf("build harass garbage obstruct\n");
        // Build fast and trigger chain fast
        return AI::think_harass(field, queue, Eval::FAST_WEIGHT, (data.all_clear) ? 10 : 2100);
    }

    // Else, if our enemy's field is smaller than our's field a lot, then harass them
    if (AI::get_small_field(enemy.field, field)) {
        // printf("build harass small field\n");
        // Trigger harassment
        return AI::think_harass(field, queue, w, (data.all_clear) ? 10 : 420);
    }

    // Try to harass
    if (field.get_count() >= 48 && field.get_count() < 62) {
        i32 enemy_attack = Detect::detect(enemy.field).main.chain.score + enemy.all_clear * 30 * data.target;

        u8 enemy_heights[6];
        enemy.field.get_heights(enemy_heights);

        i32 enemy_height_diff = *std::max_element(enemy_heights, enemy_heights + 6) - *std::min_element(enemy_heights, enemy_heights + 6);

        if (enemy_attack < 840 || enemy_heights[2] > 10 || enemy_height_diff <= 1) {
            std::vector<std::pair<i32, i32>> candidate_attack;

            for (i32 i = 0; i < search_attacks.candidates.size(); ++i) {
                if (search_attacks.candidates[i].attacks[0].frame != 0) {
                    continue;
                }

                auto attack_harass = search_attacks.candidates[i].attacks[0];

                if (attack_harass.count < 4 && attack_harass.score + data.bonus + data.all_clear * 30 * data.target >= 420) {
                    auto field_copy = field;

                    field_copy.drop_pair(search_attacks.candidates[i].placement.x, search_attacks.candidates[i].placement.r, queue[0]);
                    field_copy.pop();

                    candidate_attack.push_back({
                        i,
                        Eval::evaluate(field_copy, Detect::detect(field_copy), 0, w)
                    });
                }
            }

            if (!candidate_attack.empty()) {
                auto best = *std::max_element(
                    candidate_attack.begin(),
                    candidate_attack.end(),
                    [&] (const std::pair<i32, i32>& a, const std::pair<i32, i32>& b) {
                        return a.second < b.second;
                    }
                );

                printf("try harass\n");

                return Result {
                    .placement = search_attacks.candidates[best.first].placement,
                    .eval = best.second
                };
            }
        }
    }

    // Else, build chain normally
    return AI::think_1p(field, queue, w);
};

Result think_harass(Field field, std::vector<Cell::Pair> queue, Eval::Weight w, i32 trigger_score)
{
    auto search_result = Search::search(field, queue, w);

    if (search_result.candidates.empty()) {
        return Result {
            .placement = { .x = 2, .r = Direction::Type::UP },
            .eval = -1
        };
    }

    return AI::build(search_result, field, trigger_score, true);
};

Result build(Search::Result& search_result, Field& field, i32 trigger_score, bool harass)
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
    for (auto& candidate : search_result.candidates) {
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

                if (a.eval_fast != b.eval_fast) {
                    return a.eval_fast < b.eval_fast;
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
        if (!harass) {
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
        else {
            std::vector<std::pair<i32, Search::Attack>> attacks;

            for (i32 i = 0; i < search_result.candidates.size(); ++i) {
                for (auto attack : search_result.candidates[i].attacks) {
                    if (attack.score >= trigger_score) {
                        attacks.push_back({ i, attack });
                    }
                }
            }

            auto best = *std::max_element(
                attacks.begin(),
                attacks.end(),
                [&] (const std::pair<i32, Search::Attack>& a, const std::pair<i32, Search::Attack>& b) {
                    if (search_result.candidates[a.first].eval != search_result.candidates[b.first].eval) {
                        return search_result.candidates[a.first].eval < search_result.candidates[b.first].eval;
                    }
                    
                    if (a.second.count != b.second.count) {
                        return a.second.count > b.second.count;
                    }

                    return a.second.frame > b.second.frame;
                }
            );

            return Result {
                .placement = search_result.candidates[best.first].placement,
                .eval = search_result.candidates[best.first].eval
            };
        }
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

bool get_garbage_obstruct(Field& field, std::vector<Cell::Pair>& queue)
{
    i32 unburied_count = AI::get_unburied_count(field);
    i32 garbage_count = field.data[static_cast<i32>(Cell::Type::GARBAGE)].get_count();

    if (garbage_count < 1) {
        return false;
    }

    // i32 attack = Detect::detect(field).main.chain.score;

    return
        (garbage_count >= (field.get_count() / 2)) ||
        (unburied_count <= field.get_count() / 2 && garbage_count >= 12);
        // (unburied_count <= field.get_count() / 2) ||
        // (garbage_count > 12 && attack <= 360);
};

bool get_small_field(Field& field, Field& other)
{
    return other.get_count() > field.get_count() * 2;
};

bool get_harassable(Data& data, Enemy& enemy, Field& field)
{
    if (field.get_count() < 48 || field.get_count() >= 62) {
        return false;
    }

    i32 enemy_attack = Detect::detect(enemy.field).main.chain.score + enemy.all_clear * 30 * data.target;

    u8 enemy_heights[6];
    enemy.field.get_heights(enemy_heights);
    i32 enemy_height_diff = *std::max_element(enemy_heights, enemy_heights + 6) - *std::min_element(enemy_heights, enemy_heights + 6);

    return
        (enemy_attack <= 360 && field.get_count() >= 48 && field.get_count() <= 62) ||
        (enemy_attack <= 360 && (field.get_count() >= enemy.field.get_count() * 2)) ||
        (enemy_attack <= 360 && enemy.field.get_height(2) >= 11) ||
        (enemy_attack <= 360 && enemy_height_diff <= 2);
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