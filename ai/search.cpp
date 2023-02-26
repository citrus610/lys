#include "search.h"

namespace Search
{

Result search
(
    Field field,
    std::vector<Cell::Pair> queue,
    Eval::Weight w
)
{
    if (queue.size() <= 2) {
        return Result();
    }

    Result result = Result();

    TTable<i32> table = TTable<i32>();
    table.init();

    std::vector<std::pair<Candidate, Node>> candidates = {};

    auto placements = Move::generate(field, queue[0].first == queue[0].second);
    for (i32 i = 0; i < placements.get_size(); ++i) {
        Candidate candidate = Candidate {
            .placement = placements[i],
            .attacks = std::vector<Attack>(),
            .eval = INT32_MIN,
        };

        Field child = field;
        child.drop_pair(placements[i].x, placements[i].r, queue[0]);
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

        i32 frame_drop = field.get_drop_pair_frame(placements[i].x, placements[i].r);
        
        candidates.push_back({
            candidate,
            Node {
                .field = child,
                .score = chain.score,
                .frame = frame_drop + chain.count * 2,
                .eval = Eval::evaluate(
                    child,
                    std::nullopt,
                    frame_drop + chain.count * 2,
                    w
                )
            }
        });
    }

    std::sort(
        candidates.begin(),
        candidates.end(),
        [&] (const std::pair<Candidate, Node>& a, const std::pair<Candidate, Node>& b) {
            return a.second.eval > b.second.eval;
        }
    );

    for (auto& c : candidates) {
        // std::cin.get();
        // c.second.field.print();
        // std::cout << "placement: " << i32(c.first.placement.x) << " " << static_cast<i32>(c.first.placement.r) << "\n";
        // std::cout << "score: " << c.second.score << "\n";
        // std::cout << "frame: " << c.second.frame << "\n";
        // std::cout << "eval: " << c.second.eval << "\n" << std::endl;

        c.first.eval = best_first_search(
            c.second.field,
            queue,
            w,
            table,
            c.first.attacks,
            c.second.score,
            c.second.frame,
            1
        );

        if (c.first.eval == INT32_MIN && c.first.attacks.empty()) {
            continue;
        }

        result.candidates.push_back(c.first);
    }

    return result;
};

i32 best_first_search
(
    Field& field,
    std::vector<Cell::Pair>& queue,
    Eval::Weight& w,
    TTable<i32>& table,
    std::vector<Attack>& attacks,
    i32 score,
    i32 frame,
    i32 depth
)
{
    i32 result = INT32_MIN;

    avec<Node, 22> nodes = avec<Node, 22>();

    auto placements = Move::generate(field, queue[depth].first == queue[depth].second);
    for (i32 i = 0; i < placements.get_size(); ++i) {
        Node child = Node {
            .field = field,
            .score = score,
            .frame = frame,
            .eval = INT32_MIN
        };

        child.field.drop_pair(placements[i].x, placements[i].r, queue[depth]);
        auto mask_pop = child.field.pop();

        if (child.field.get_height(2) > 11) {
            continue;
        }

        auto chain = Chain::get_score(mask_pop);

        if (chain.count > 0) {
            attacks.push_back({
                .score = chain.score,
                .count = chain.count,
                .frame = frame,
                .all_clear = child.field.is_empty()
            });
        }

        child.frame += field.get_drop_pair_frame(placements[i].x, placements[i].r) + chain.count * 2;
        child.score += chain.score;
        child.eval = Eval::evaluate(child.field, std::nullopt, child.frame, w);

        nodes.add(child);
    }

    std::sort(
        nodes.iter_begin(),
        nodes.iter_end(),
        [&] (const Node& a, const Node& b) {
            return a.eval > b.eval;
        }
    );

    for (i32 i = 0; i < nodes.get_size(); ++i) {
        // nodes[i].field.print();
        // std::cout << "score: " << nodes[i].score << "\n";
        // std::cout << "frame: " << nodes[i].frame << "\n";
        // std::cout << "eval: " << nodes[i].eval << "\n" << std::endl;

        i32 eval = INT32_MIN;
        u64 hash = table.get_hash(nodes[i].field, depth);

        if (!table.get_entry(hash, eval)) {
            if (depth + 1 < queue.size()) {
                eval = best_first_search(
                    nodes[i].field,
                    queue,
                    w,
                    table,
                    attacks,
                    nodes[i].score,
                    nodes[i].frame,
                    depth + 1
                );
            }
            else {
                auto detect = Detect::detect(nodes[i].field);
                eval = Eval::evaluate(nodes[i].field, std::max(detect.flat, detect.deep), nodes[i].frame, w);
                if (detect.flat.chain.count > 0) {
                    attacks.push_back({
                        .score = detect.flat.chain.score,
                        .count = detect.flat.chain.count,
                        .frame = nodes[i].frame,
                        .all_clear = false
                    });
                }
            }

            table.set_entry(hash, eval);
        }

        result = std::max(result, eval);
    }

    return result;
};

};