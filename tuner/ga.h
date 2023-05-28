#pragma once

#include "data.h"
#include "score.h"

#include <iostream>
#include <string>
#include <random>

class GeneticAl
{
public:
    std::vector<Eval::Gene> pool;
    int gen_id;
public:
    void save();
    void load(int id);
public:
    static int randomize(int a, int delta, int max = 1000);
    static int mutate(int a, int b, int max = 1000);
public:
    static Eval::Weight randomize(Eval::Weight h, int delta = 100);
    static Eval::Weight mutate(Eval::Weight& a, Eval::Weight& b);
public:
    void init(Eval::Weight a, int size);
    void simulate();
};

#define GA_RANDOMIZE(p, d, max) result.p = GeneticAl::randomize(h.p, d, max);
#define GA_MUTATE(a, b, p, max) result.p = GeneticAl::mutate(a.p, b.p, max);