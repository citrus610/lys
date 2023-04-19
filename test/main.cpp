#include <iostream>
#include "../ai/ai.h"
#include "../core/core.h"
#include "../ai/eval.h"
#include "../ai/search.h"

void load_json_heuristic(Eval::Weight& h)
{
    std::ifstream file;
    file.open("config.json");
    json js;
    file >> js;
    file.close();
    from_json(js, h);
};

void save_json_heuristic()
{
    std::ifstream f("config.json");
    if (f.good()) {
        return;
    };
    f.close();

    std::ofstream o("config.json");
    json js;
    to_json(js, Eval::DEFAULT_WEIGHT);
    o << std::setw(4) << js << std::endl;
    o.close();
};

static Chain::Score get_score(std::vector<Cell::Pair> queue, Eval::Weight w)
{
    Field field = Field();

    int skim_count = 0;

    for (int i = 0; i < 128; ++i)
    {
        std::vector<Cell::Pair> tqueue;
        tqueue.push_back(queue[(i + 0) % queue.size()]);
        tqueue.push_back(queue[(i + 1) % queue.size()]);
        tqueue.push_back(queue[(i + 2) % queue.size()]);

        AI::Result airesult = AI::think_1p(field, tqueue, w);

        field.drop_pair(airesult.placement.x, airesult.placement.r, tqueue[0]);

        auto mask = field.pop();
        auto chain = Chain::get_score(mask);

        if (field.get_height(2) > 11) {
            break;
        }

        if (chain.count > 5 || chain.score > 7000) {
            return Chain::Score { .count = chain.count, .score = chain.score };
        }
    }

    return Chain::Score { .count = 0, .score = 0 };
};

std::vector<Cell::Pair> create_queue()
{
    using namespace std;

    srand(uint32_t(time(NULL)));

    vector<Cell::Type> bag;
    bag.reserve(256);

    for (int i = 0; i < 64; ++i) {
        for (uint8_t p = 0; p < Cell::COUNT - 1; ++p) {
            bag.push_back(Cell::Type(p));
        }
    }

    for (int t = 0; t < 4; ++t) {
        for (int i = 0; i < 256; ++i) {
            int k = rand() % 256;
            Cell::Type value = bag[i];
            bag[i] = bag[k];
            bag[k] = value;
        }
    }

    vector<Cell::Pair> queue;
    queue.reserve(128);

    for (int i = 0; i < 128; ++i) {
        queue.push_back({ bag[i * 2], bag[i * 2 + 1] });
    }

    return queue;
};

int main()
{
    // srand(uint32_t(time(NULL)));

    // Field f;
    // const char c[13][7] = {
    //     "......",
    //     "......",
    //     "......",
    //     "......",
    //     "......",
    //     "......",
    //     "......",
    //     "......",
    //     "......",
    //     "......",
    //     "RYG...",
    //     "RRYG..",
    //     "YYGG..",
    // };
    // f.from(c);
    // f.print();

    // i32 l2, l3, lm;
    // Eval::link(f, l2, l3, lm);

    // printf("l2: %d, l3: %d, lm: %d\n", l2, l3, lm);

    // auto detect = Detect::detect(f);
    // printf("main:\n    count: %d\n    score: %d\n    height: %d\n    needed: %d\n\n", detect.main.chain.count, detect.main.chain.score, detect.main.height, detect.main.needed);
    // printf("sub:\n    count: %d\n    score: %d\n    height: %d\n    needed: %d\n\n", detect.harass.chain.count, detect.harass.chain.score, detect.harass.height, detect.harass.needed);

    // detect = Detect::detect_deep(f);
    // printf("main:\n    count: %d\n    score: %d\n    height: %d\n    needed: %d\n\n", detect.main.chain.count, detect.main.chain.score, detect.main.height, detect.main.needed);
    // printf("sub:\n    count: %d\n    score: %d\n    height: %d\n    needed: %d\n\n", detect.harass.chain.count, detect.harass.chain.score, detect.harass.height, detect.harass.needed);

    // auto q = create_queue();

    // std::cout << Cell::to_char(q[0].first) << Cell::to_char(q[0].second) << " ";
    // std::cout << Cell::to_char(q[1].first) << Cell::to_char(q[1].second) << " ";
    // std::cout << Cell::to_char(q[2].first) << Cell::to_char(q[2].second) << "\n";

    // auto time_1 = std::chrono::high_resolution_clock::now();
    // auto attack = Search::Attack::search(f, { q[0], q[1], q[2] });
    // auto time_2 = std::chrono::high_resolution_clock::now();
    // auto result = Search::Beam::search(f, { q[0], q[1], q[2] });
    // auto time_3 = std::chrono::high_resolution_clock::now();
    // auto time_a = std::chrono::duration_cast<std::chrono::microseconds>(time_2 - time_1).count();
    // printf("time attack: %d\n", time_a);
    // auto time_b = std::chrono::duration_cast<std::chrono::microseconds>(time_3 - time_2).count();
    // printf("time beam: %d\n", time_b);
    // printf("time total: %d\n", time_a + time_b);

    // result.plan.print();
    // printf("x: %d, r: %d, eval: %d\n", result.placement.x, static_cast<i32>(result.placement.r), result.eval);

    // for (auto& c : attack.candidates) {
    //     auto ff = f;
    //     ff.drop_pair(c.placement.x, c.placement.r, q[0]);
    //     ff.pop();
    //     ff.print();

    //     printf("x: %d, r: %d, attack: %d\n", c.placement.x, static_cast<i32>(c.placement.r), c.attacks.size());
    // }

    srand(uint32_t(time(NULL)));

    std::atomic<i32> map_count[20] = { 0 };
    std::atomic<i32> map_score[20] = { 0 };

    Eval::Weight w;
    save_json_heuristic();
    load_json_heuristic(w);

    std::mutex mt;

    const i32 sim_count = 1000;
    std::atomic<i32> i = 0;

    std::vector<std::thread> threads;
    for (i32 t = 0; t < 4; ++t) {
        threads.emplace_back([&] () {
            while (i < sim_count)
            {
                auto queue = create_queue();
                auto score = get_score(queue, w);
                map_count[score.count] += 1;
                map_score[score.score / 10000] += 1;
                i++;
                printf("\rprocess: %d/%d", i.load(), sim_count);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    printf("\n");
    printf("AI chain count distribution (%d simulations):\n", sim_count);
    for (i32 k = 0; k < 20; ++k) {
        printf("    %d: %d\n", k, map_count[k].load());
    }

    printf("\n");
    printf("AI chain score distribution (%d simulations):\n", sim_count);
    for (i32 k = 0; k < 20; ++k) {
        printf("    %d - %d: %d\n", k * 10000, (k + 1) * 10000, map_score[k].load());
    }

    return 0;
};