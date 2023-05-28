#include "ga.h"

int GeneticAl::randomize(int a, int delta, int max)
{
    int c = (rand() % (delta * 2 + 1)) - delta;
    // int c = a + (rand() % (delta * 2 + 1)) - delta;
    c += (rand() % 3) - 1;
    c = std::min(c, max);
    c = std::max(c, -max);
    return c;
};

int GeneticAl::mutate(int a, int b, int max)
{
    int rn = rand() % 100;
    int c = 0;
    if (rn < 45) {
        c = a;
    }
    else if (rn < 90) {
        c = b;
    }
    else {
        // c = (a + b) / 2;
        c = (rand() % (max * 2 + 1)) - max;
    }
    c += (rand() % 3) - 2;
    c = std::min(c, max);
    c = std::max(c, -max);
    return c;
};

Eval::Weight GeneticAl::randomize(Eval::Weight h, int delta)
{
    Eval::Weight result = h;

    GA_RANDOMIZE(diff, 100, 100)
    GA_RANDOMIZE(diff_s, 50, 50)
    GA_RANDOMIZE(well, 50, 50)
    GA_RANDOMIZE(well_s, 50, 50)
    GA_RANDOMIZE(shape, 100, 100)
    GA_RANDOMIZE(shape_s, 100, 100)
    result.nuisance = -100;
    GA_RANDOMIZE(side_bias, 100, 100)
    GA_RANDOMIZE(disconnect, 200, 200)
    GA_RANDOMIZE(frame, 100, 100)
    GA_RANDOMIZE(chain_count, 1500, 1500)
    GA_RANDOMIZE(chain_score, 100, 100)
    GA_RANDOMIZE(chain_height, 100, 100)
    GA_RANDOMIZE(chain_needed, 100, 100)


    return result;
};

Eval::Weight GeneticAl::mutate(Eval::Weight& a, Eval::Weight& b)
{
    Eval::Weight result;

    // i32 diff = 0;
    // i32 diff_s = 0;
    // i32 well = 0;
    // i32 well_s = 0;
    // i32 shape = 0;
    // i32 shape_s = 0;
    // i32 nuisance = 0;
    // i32 side_bias = 0;
    // i32 disconnect = 0;
    // i32 frame = 0;
    // i32 chain_count = 0;
    // i32 chain_score = 0;
    // i32 chain_height = 0;
    // i32 chain_needed = 0;

    GA_MUTATE(a, b, diff, 100)
    GA_MUTATE(a, b, diff_s, 50)
    GA_MUTATE(a, b, well, 50)
    GA_MUTATE(a, b, well_s, 50)
    GA_MUTATE(a, b, shape, 100)
    GA_MUTATE(a, b, shape_s, 100)
    GA_MUTATE(a, b, nuisance, 1000)
    GA_MUTATE(a, b, side_bias, 100)
    GA_MUTATE(a, b, disconnect, 200)
    GA_MUTATE(a, b, frame, 100)
    GA_MUTATE(a, b, chain_count, 1500)
    GA_MUTATE(a, b, chain_score, 1000)
    GA_MUTATE(a, b, chain_height, 100)
    GA_MUTATE(a, b, chain_needed, 100)

    return result;
};

void GeneticAl::init(Eval::Weight a, int size)
{
    this->gen_id = 0;
    this->pool.clear();
    this->pool.reserve(size);

    for (int i = 0; i < size; ++i) {
        this->pool.push_back({ .heuristic = GeneticAl::randomize(a), .score = 0.0 });
    }

    this->save();
};

void GeneticAl::simulate()
{
    std::sort
    (
        this->pool.begin(),
        this->pool.end(),
        [&] (const Eval::Gene& a, const Eval::Gene& b) {
            return b.score < a.score;
        }
    );

    int full_size = this->pool.size();

    for (int i = 0; i < full_size / 2; ++i) {
        this->pool.pop_back();
    }

    int half_size = this->pool.size();

    for (int i = 0; i < half_size - 1; ++i) {
        this->pool.push_back
        (
            { 
                .heuristic = GeneticAl::mutate(this->pool[i].heuristic, this->pool[i + 1].heuristic), 
                .score = 0.0
            }
        );
    }

    this->pool.push_back
    (
        {
            .heuristic = GeneticAl::mutate(this->pool[0].heuristic, this->pool[half_size - 1].heuristic),
            .score = 0.0
        }
    );

    for (int i = 0; i < this->pool.size(); ++i) {
        this->pool[i].score = 0.0;
    }

    this->gen_id += 1;

    this->save();
};

void GeneticAl::save()
{
    Eval::SaveData save_data;
    save_data.data = this->pool;
    tuner_save_json(save_data, this->gen_id);
};

void GeneticAl::load(int id)
{
    Eval::SaveData save_data;
    tuner_load_json(save_data, id);
    this->pool = save_data.data;
    this->gen_id = id;
};