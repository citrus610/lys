#pragma once

#include "../ai/ai.h"
#include "spsa.h"

static std::vector<Cell::Pair> create_queue()
{
    using namespace std;

    vector<Cell::Type> bag;
    bag.reserve(256);

    for (int i = 0; i < 64; ++i) {
        for (uint8_t p = 0; p < static_cast<uint8_t>(Cell::COUNT) - 1; ++p) {
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

static Eval::Weight theta_to_weight(std::vector<double> theta)
{
    Eval::Weight w = Eval::Weight();

    w.chain_height = i32(std::round(theta[0] * 100.0));
    w.chain_needed = i32(std::round(theta[1] * 100.0));
    w.link_hor = i32(std::round(theta[2] * 100.0));
    w.link_ver = i32(std::round(theta[3] * 100.0));
    w.frame = i32(std::round(theta[4] * 100.0));
    w.chain_count = 1000;
    w.shape = -100;

    return w;
};

static std::vector<double> weight_to_theta(Eval::Weight w)
{
    std::vector<double> theta;

    theta.push_back(double(w.chain_height) / 100.0);
    theta.push_back(double(w.chain_needed) / 100.0);
    theta.push_back(double(w.link_hor) / 100.0);
    theta.push_back(double(w.link_ver) / 100.0);
    theta.push_back(double(w.frame) / 100.0);

    return theta;
};

static double get_score(std::vector<double> theta)
{
    auto w = theta_to_weight(theta);

    auto queue = create_queue();

    Field field = Field();

    double best = 0.0;

    i32 frame = 0;

    for (int i = 0; i < 128; ++i)
    {
        std::vector<Cell::Pair> tqueue;
        tqueue.push_back(queue[(i + 0) % queue.size()]);
        tqueue.push_back(queue[(i + 1) % queue.size()]);
        tqueue.push_back(queue[(i + 2) % queue.size()]);

        AI::Result airesult = AI::think_1p(field, tqueue, w);

        auto drop_frame = field.get_drop_pair_frame(airesult.placement.x, airesult.placement.r);

        field.drop_pair(airesult.placement.x, airesult.placement.r, tqueue[0]);

        auto mask = field.pop();
        auto chain = Chain::get_score(mask);

        if (field.get_height(2) > 11) {
            break;
        }

        if (chain.score > 7000 || chain.count > 5) {
            best = std::max(best, double(chain.score) / 10000.0);
            break;
        }

        frame += drop_frame + chain.count * 2;
    }

    // auto regularization = Utils::regulizer(Utils::difference(theta, weight_to_theta(Eval::DEFAULT_WEIGHT)), 0.01, 0.5);

    return -best;
};

static std::vector<double> constrain(std::vector<double> theta)
{
    theta[0] = std::max(theta[0], 0.0);
    theta[1] = std::min(theta[1], 0.0);
    theta[2] = std::max(theta[2], 0.0);
    theta[3] = std::min(theta[3], 0.0);
    theta[4] = std::min(theta[4], 0.0);

    return theta;
};