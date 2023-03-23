#include "ga.h"

int GeneticAl::randomize(int a, int delta, int max)
{
    // int c = a + (rand() % (delta * 2 + 1)) - delta;
    int c = (rand() % (delta * 2 + 1)) - delta;
    // c += (rand() % 3) - 1;
    // c = std::min(c, max);
    // c = std::max(c, -max);
    return c;
};

int GeneticAl::mutate(int a, int b, int max)
{
    int rn = rand() % 100;
    int c = 0;
    if (rn < 40) {
        c = a;
    }
    else if (rn < 80) {
        c = b;
    }
    else {
        c = (a + b) / 2;
    }
    c += (rand() % 3) - 2;
    // c = std::min(c, max);
    // c = std::max(c, -max);
    return c;
};

Eval::Weight GeneticAl::randomize(Eval::Weight h, int delta)
{
    Eval::Weight result = h;

    GA_RANDOMIZE(link_v, 1000)
    GA_RANDOMIZE(link_h, 1000)
    GA_RANDOMIZE(link_mid, 1000)
    GA_RANDOMIZE(d_height, 1000)
    GA_RANDOMIZE(d_height_sq, 100)
    GA_RANDOMIZE(symm, 1000)
    GA_RANDOMIZE(symm_sq, 100)
    GA_RANDOMIZE(shape_u, 1000)
    GA_RANDOMIZE(shape_u_sq, 100)

    GA_RANDOMIZE(frame, 50)
    GA_RANDOMIZE(ptnl_chain_count, 1000)
    GA_RANDOMIZE(ptnl_chain_score, 1000)
    GA_RANDOMIZE(ptnl_chain_needed, 1000)
    GA_RANDOMIZE(ptnl_chain_height, 1000)

    return result;
};

Eval::Weight GeneticAl::mutate(Eval::Weight& a, Eval::Weight& b)
{
    Eval::Weight result;

    GA_MUTATE(a, b, link_v)
    GA_MUTATE(a, b, link_h)
    GA_MUTATE(a, b, link_mid)
    GA_MUTATE(a, b, d_height)
    GA_MUTATE(a, b, d_height_sq)
    GA_MUTATE(a, b, symm)
    GA_MUTATE(a, b, symm_sq)
    GA_MUTATE(a, b, shape_u)
    GA_MUTATE(a, b, shape_u_sq)

    GA_MUTATE(a, b, frame)
    GA_MUTATE(a, b, ptnl_chain_count)
    GA_MUTATE(a, b, ptnl_chain_score)
    GA_MUTATE(a, b, ptnl_chain_needed)
    GA_MUTATE(a, b, ptnl_chain_height)

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