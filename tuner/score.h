#pragma once

#include "data.h"

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

static void tuner_save_json(Eval::SaveData& save_data, int gen_id)
{
    std::string gen_name = std::string("data/gen_") + std::to_string(gen_id) + std::string(".json");
    std::ofstream o(gen_name, std::ofstream::out | std::ofstream::trunc);
    json js;
    to_json(js, save_data);
    o << std::setw(4) << js << std::endl;
    o.close();
};

static void tuner_load_json(Eval::SaveData& save_data, int gen_id)
{
    std::string gen_name = std::string("data/gen_") + std::to_string(gen_id) + std::string(".json");
    std::ifstream file;
    file.open(gen_name);
    json js;
    file >> js;
    file.close();
    from_json(js, save_data);
};