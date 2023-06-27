#pragma once

#include <functional>
#include <optional>

#include "utils.h"

namespace Spsa
{

struct Options
{
    double a = 1.1;
    double c = 0.1;
    double alpha = 0.7;
    double gamma = 0.12;
    double A = 100;
};

class Minimizer
{
public:
    std::function<double(std::vector<double>)> f;
    std::vector<double> theta0;
    int iter;
    int max_iter;
    std::optional<std::function<std::vector<double>(std::vector<double>)>> constraints;
    Options options;
public:
    std::vector<double> previous_gradient;
    std::vector<double> rprop_previous_g;
    std::vector<double> rprop_previous_delta;
public:
    double history_eval[1000];
    std::vector<double> history_theta[1000];
    int history_count;
public:
    double best_eval[1000];
    std::vector<double> best_theta[1000];
    int best_count;
public:
    Minimizer(
        std::function<double(std::vector<double>)> f,
        std::vector<double> theta0,
        int max_iter,
        std::optional<std::function<std::vector<double>(std::vector<double>)>> constraints = std::nullopt,
        Options options = Options()
    );
public:
    std::vector<double> run();
    double evaluate_goal(std::vector<double> theta);
    std::vector<double> approximate_gradient(std::vector<double> theta, double c);
    std::vector<double> create_bernouilli(int size);
    std::pair<double, std::vector<double>> average_evaluations(int n);
    std::pair<double, std::vector<double>> average_best_evals(int n);
    std::vector<double> rprop(std::vector<double> theta, std::vector<double> gradient);
};

};