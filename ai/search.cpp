#include "search.h"

namespace Search
{

Result search
(
    Field field,
    std::vector<Cell::Pair> queue,
    Eval::Weight w,
    i32 thread_count
)
{
    if (queue.size() <= 2) {
        return Result();
    }

    Result result = Result();

    auto placements = Move::generate(field, queue[0].first == queue[0].second);

    std::mutex mtx;
    std::vector<std::thread> threads;

    for (i32 t = 0; t < thread_count; ++t) {
        threads.emplace_back([&] () {
            while (true)
            {
                Move::Placement placement;

                {
                    std::lock_guard<std::mutex> lk(mtx);
                    if (placements.get_size() < 1) {
                        break;
                    }
                    placement = placements[placements.get_size() - 1];
                    placements.pop();
                }

                Candidate candidate = Candidate {
                    .placement = placement,
                    .attacks = std::vector<Attack>(),
                    .eval = INT32_MIN
                };

                Field child = field;
                child.drop_pair(placement.x, placement.r, queue[0]);
                auto mask_pop = child.pop();

                if (child.get_height(2) > 11) {
                    continue;
                }

                auto chain = Chain::get_score(mask_pop);

                if (chain.count > 0) {
                    candidate.attacks.push_back({
                        .score = chain.score,
                        .count = chain.count,
                        .frame = 0,
                        .all_clear = child.is_empty()
                    });
                }

                candidate.eval = Search::bfs(
                    child,
                    queue,
                    w,
                    candidate.attacks,
                    chain.score,
                    field.get_drop_pair_frame(placement.x, placement.r) + chain.count * 2,
                    1
                );

                if (candidate.eval == INT32_MIN && candidate.attacks.empty()) {
                    continue;
                }

                {
                    std::lock_guard<std::mutex> lk(mtx);
                    result.candidates.push_back(std::move(candidate));
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    return result;
};

i32 bfs
(
    Field& field,
    std::vector<Cell::Pair>& queue,
    Eval::Weight& w,
    std::vector<Attack>& attacks,
    i32 score,
    i32 frame,
    i32 depth
)
{
    i32 result = INT32_MIN;

    auto placements = Move::generate(field, queue[depth].first == queue[depth].second);

    for (i32 i = 0; i < placements.get_size(); ++i) {
        Field child = field;
        child.drop_pair(placements[i].x, placements[i].r, queue[depth]);
        auto mask_pop = child.pop();

        if (child.get_height(2) > 11) {
            continue;
        }

        auto chain = Chain::get_score(mask_pop);

        if (chain.count > 0) {
            attacks.push_back({
                .score = chain.score,
                .count = chain.count,
                .frame = frame,
                .all_clear = child.is_empty()
            });
        }

        i32 eval = INT32_MIN;

        if (depth + 1 < queue.size()) {
            eval = Search::bfs(
                child,
                queue,
                w,
                attacks,
                score + chain.score,
                frame + field.get_drop_pair_frame(placements[i].x, placements[i].r) + chain.count * 2,
                depth + 1
            );
        }
        else {
            eval = Eval::evaluate(
                child,
                Detect::detect(child),
                frame + field.get_drop_pair_frame(placements[i].x, placements[i].r) + chain.count * 2,
                w
            );
        }

        result = std::max(result, eval);
    }

    return result;
};

Result search_attacks
(
    Field field,
    std::vector<Cell::Pair> queue,
    bool fast,
    i32 thread_count
)
{
    if (queue.size() <= 2) {
        return Result();
    }

    Result result = Result();

    auto placements = Move::generate(field, queue[0].first == queue[0].second);

    std::mutex mtx;
    std::vector<std::thread> threads;

    for (i32 t = 0; t < thread_count; ++t) {
        threads.emplace_back([&] () {
            while (true)
            {
                Move::Placement placement;

                {
                    std::lock_guard<std::mutex> lk(mtx);
                    if (placements.get_size() < 1) {
                        break;
                    }
                    placement = placements[placements.get_size() - 1];
                    placements.pop();
                }

                Candidate candidate = Candidate {
                    .placement = placement,
                    .attacks = std::vector<Attack>(),
                    .eval = INT32_MIN
                };

                candidate.attacks.reserve(512);

                Field child = field;
                child.drop_pair(placement.x, placement.r, queue[0]);
                auto mask_pop = child.pop();

                if (child.get_height(2) > 11) {
                    continue;
                }

                auto chain = Chain::get_score(mask_pop);

                if (chain.count > 0) {
                    candidate.attacks.push_back({
                        .score = chain.score,
                        .count = chain.count,
                        .frame = 0,
                        .all_clear = child.is_empty()
                    });
                }

                Search::bfs_attacks(
                    child,
                    queue,
                    candidate.attacks,
                    fast,
                    chain.score,
                    field.get_drop_pair_frame(placement.x, placement.r) + chain.count * 2,
                    1
                );

                if (candidate.attacks.empty()) {
                    continue;
                }

                {
                    std::lock_guard<std::mutex> lk(mtx);
                    result.candidates.push_back(std::move(candidate));
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    return result;
};

void bfs_attacks
(
    Field& field,
    std::vector<Cell::Pair>& queue,
    std::vector<Attack>& attacks,
    bool fast,
    i32 score,
    i32 frame,
    i32 depth
)
{
    auto placements = Move::generate(field, queue[depth].first == queue[depth].second);

    for (i32 i = 0; i < placements.get_size(); ++i) {
        Field child = field;
        child.drop_pair(placements[i].x, placements[i].r, queue[depth]);
        auto mask_pop = child.pop();

        if (child.get_height(2) > 11) {
            continue;
        }

        auto chain = Chain::get_score(mask_pop);

        if (chain.count > 0) {
            attacks.push_back({
                .score = chain.score + score,
                .count = chain.count,
                .frame = frame,
                .all_clear = child.is_empty()
            });
        }

        if (depth + 1 < queue.size()) {
            Search::bfs_attacks(
                child,
                queue,
                attacks,
                fast,
                score + chain.score,
                frame + field.get_drop_pair_frame(placements[i].x, placements[i].r) + chain.count * 2,
                depth + 1
            );
        }
        else if (!fast) {
            auto detect = Detect::detect_fast(child);
            if (detect.main.chain.count > 0) {
                attacks.push_back({
                    .score = detect.main.chain.score + score,
                    .count = detect.main.chain.count,
                    .frame = frame + field.get_drop_pair_frame(placements[i].x, placements[i].r) + chain.count * 2 + detect.main.needed - 1,
                    .all_clear = false
                });
            }
        }
    }
};

};