#include "search.h"

namespace Search
{

namespace Beam
{

Result search(
    Field field,
    std::vector<Cell::Pair> queue,
    Eval::Weight w
)
{
    if (queue.size() <= 2) {
        return Result();
    }

    Result result = Result();

    Layer layers[2] = { Layer(), Layer() };
    layers[0].init(50);
    layers[1].init(50);

    auto placements = Move::generate(field, queue[0].first == queue[0].second);

    for (i32 i = 0; i < placements.get_size(); ++i) {
        Node node = Node {
            .field = field,
            .frame = 0,
            .index = i,
            .eval = INT32_MIN
        };

        node.field.drop_pair(placements[i].x, placements[i].r, queue[0]);
        auto mask_pop = node.field.pop();

        if (node.field.get_height(2) > 11) {
            continue;
        }

        node.frame = field.get_drop_pair_frame(placements[i].x, placements[i].r) + mask_pop.get_size() * 2;

        // node.eval = Eval::evaluate(node.field, std::nullopt, node.frame, w);
        // node.eval = Eval::evaluate(node.field, Detect::detect(node.field), node.frame, w);
        node.eval = Eval::evaluate(node.field, Detect::detect_deep(node.field), node.frame, w);

        layers[0].add(node);
    }

    if (layers[0].data.empty()) {
        return Result();
    }

    i32 depth = 1;
    
    for (size_t i = 1; i < queue.size(); ++i) {
        layers[(depth - 1) & 1].sort();

        for (auto& node : layers[(depth - 1) & 1].data) {
            Search::Beam::expand(
                node,
                queue[depth],
                layers[depth & 1],
                w
            );
        }

        layers[(depth - 1) & 1].clear();

        depth++;
    }

    if (layers[(depth - 1) & 1].data.empty()) {
        return Result();
    }

    // for (auto& node : layers[(depth - 1) & 1].data) {
    //     node.eval = Eval::evaluate(node.field, Detect::detect_deep(node.field), node.frame, w);
    // }

    std::sort(
        layers[(depth - 1) & 1].data.begin(),
        layers[(depth - 1) & 1].data.end(),
        [&] (Node& a, Node& b) { return b < a; }
    );

    auto best = layers[(depth - 1) & 1].data[0];

    return Result {
        .placement = placements[best.index],
        .plan = best.field,
        .eval = best.eval
    };
};

void expand
(
    Node& node,
    Cell::Pair pair,
    Layer& layer,
    Eval::Weight w
)
{
    auto placements = Move::generate(node.field, pair.first == pair.second);

    for (i32 i = 0; i < placements.get_size(); ++i) {
        Node child = node;

        child.field.drop_pair(placements[i].x, placements[i].r, pair);
        auto mask_pop = child.field.pop();

        if (child.field.get_height(2) > 11) {
            continue;
        }

        child.frame = node.field.get_drop_pair_frame(placements[i].x, placements[i].r) + mask_pop.get_size() * 2;

        // child.eval = Eval::evaluate(child.field, std::nullopt, child.frame, w);
        // child.eval = Eval::evaluate(child.field, Detect::detect(child.field), child.frame, w);
        child.eval = Eval::evaluate(child.field, Detect::detect_deep(child.field), child.frame, w);

        layer.add(child);
    }
};

};

namespace Attack
{

Result search
(
    Field field,
    std::vector<Cell::Pair> queue,
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
                    .attacks = std::vector<Attack>()
                };

                candidate.attacks.reserve(1000);

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

                Search::Attack::dfs(
                    child,
                    queue,
                    candidate.attacks,
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

void dfs
(
    Field& field,
    std::vector<Cell::Pair>& queue,
    std::vector<Attack>& attacks,
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

        i32 frame_drop = field.get_drop_pair_frame(placements[i].x, placements[i].r);

        if (depth + 1 < queue.size()) {
            Search::Attack::dfs(
                child,
                queue,
                attacks,
                score + chain.score,
                frame + frame_drop + chain.count * 2,
                depth + 1
            );
        }
        else {
            auto detect = Detect::detect(child);
            if (detect.harass.chain.count > 0) {
                attacks.push_back({
                    .score = detect.harass.chain.score + chain.score + score,
                    .count = detect.harass.chain.count,
                    .frame = frame + frame_drop + chain.count * 2,
                    .all_clear = false
                });
            }
            if (detect.main.chain.count > 0) {
                attacks.push_back({
                    .score = detect.main.chain.score + chain.score + score,
                    .count = detect.main.chain.count,
                    .frame = frame + frame_drop + chain.count * 2,
                    .all_clear = false
                });
            }
        }
    }
};

};

};