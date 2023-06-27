#include "spsa.h"

namespace Spsa
{

Minimizer::Minimizer(
    std::function<double(std::vector<double>)> f,
    std::vector<double> theta0,
    int max_iter,
    std::optional<std::function<std::vector<double>(std::vector<double>)>> constraints,
    Options options
)
{
    this->f = f;
    this->theta0 = theta0;
    this->iter = 0;
    this->max_iter = max_iter;
    this->constraints = constraints;
    this->options = options;

    this->previous_gradient.clear();
    this->rprop_previous_g.clear();
    this->rprop_previous_delta.clear();

    this->history_count = 0;

    this->best_count = 0;

    this->options.A = double(max_iter) / 10.0;

    printf("done construct\n");
};

std::vector<double> Minimizer::run()
{
    int k = 0;
    auto theta = this->theta0;

    while (true)
    {
        k += 1;

        this->iter = k;

        if (this->constraints.has_value()) {
            theta = this->constraints.value()(theta);
        }

        printf("iter: %d %s theta: %s\n", k, std::string(6 - std::to_string(k).size(), ' ').c_str(), Utils::pretty(theta).c_str());

        auto c_k = this->options.c / std::pow(double(k), this->options.gamma);
        auto a_k = this->options.a / std::pow(double(k) + this->options.A, this->options.alpha);

        auto gradient = this->approximate_gradient(theta, c_k);

        auto mu = -0.01 / std::max(1.0, Utils::norm2(gradient));

        theta = Utils::linear_combinaison(theta, gradient, 1.0, mu);
        theta = Utils::linear_combinaison(theta, this->rprop(theta, gradient), 1.0, -0.01);

        auto avg_best_eval = this->average_best_evals(30);

        theta = Utils::linear_combinaison(theta, avg_best_eval.second, 0.98, 0.02);

        if (k >= this->max_iter) {
            break;
        }
    }

    printf("\n");

    return theta;
};

double Minimizer::evaluate_goal(std::vector<double> theta)
{
    auto v = this->f(theta);

    this->history_eval[this->history_count % 1000] = v;
    this->history_theta[this->history_count % 1000] = theta;
    this->history_count += 1;

    return v;
};

std::vector<double> Minimizer::approximate_gradient(std::vector<double> theta, double c)
{
    double current_goal = 100000000000000000.0;

    if (this->history_count > 0) {
        current_goal = this->average_evaluations(30).first;
    }

    auto bernouilli = this->create_bernouilli(theta.size());

    int count = 0;

    double f1;
    double f2;
    std::vector<double> theta1;
    std::vector<double> theta2;

    while (true)
    {
        auto seed = time(NULL);

        srand(seed);
        theta1 = Utils::linear_combinaison(theta, bernouilli, 1.0, c);
        f1 = this->evaluate_goal(theta1);

        srand(seed);
        theta2 = Utils::linear_combinaison(theta, bernouilli, 1.0, -c);
        f2 = this->evaluate_goal(theta2);

        if (f1 != f2) {
            break;
        }

        count += 1;
        if (count >= 100) {
            break;
        }
    }

    std::vector<double> gradient = {};

    for (auto i = 0; i < theta.size(); ++i) {
        gradient.push_back((f1 - f2) / (2.0 * c * bernouilli[i]));
    }

    // printf("f1: %s\n", std::to_string(f1).c_str());
    // printf("f2: %s\n", std::to_string(f2).c_str());

    if ((f1 > current_goal) && (f2 > current_goal)) {
        // printf("func is not decreasing");
        gradient = Utils::linear_combinaison(gradient, {}, 0.1, 0.0);
    }

    auto beta = 0.9;
    auto correction = 1.0 / 1.0 - std::pow(beta, this->iter);

    gradient = Utils::linear_combinaison(gradient, this->previous_gradient, (1.0 - beta), beta);
    gradient = Utils::linear_combinaison(gradient, {}, correction, 0.0);

    this->previous_gradient = gradient;

    if (f1 <= current_goal) {
        this->best_eval[this->best_count % 1000] = f1;
        this->best_theta[this->best_count % 1000] = theta1;
        this->best_count += 1;
    }

    if (f2 <= current_goal) {
        this->best_eval[this->best_count % 1000] = f2;
        this->best_theta[this->best_count % 1000] = theta2;
        this->best_count += 1;
    }

    return gradient;
};

std::vector<double> Minimizer::create_bernouilli(int size)
{
    srand(time(NULL));

    std::vector<double> bernouilli = {};

    for (int i = 0; i < size; ++i) {
        bernouilli.push_back((rand() % 2) * 2 - 1);
    }

    auto g = Utils::norm2(this->previous_gradient);
    auto d = Utils::norm2(bernouilli);

    if (g > 0.00001) {
        bernouilli = Utils::linear_combinaison(bernouilli, this->previous_gradient, 0.55, 0.25 * d / g);
    }

    for (auto i = 0; i < bernouilli.size(); ++i) {
        if (bernouilli[i] == 0.0) {
            bernouilli[i] = 0.2;
        }

        if (std::abs(bernouilli[i]) < 0.2) {
            bernouilli[i] = 0.2 * Utils::sign_of(bernouilli[i]);
        }
    }

    return bernouilli;
};

std::pair<double, std::vector<double>> Minimizer::average_evaluations(int n)
{
    if (n <= 0) n = 1;
    if (n > 1000) n = 1000;
    if (n > this->history_count) n = this->history_count;

    auto sum_eval = 0.0;
    auto sum_theta = Utils::linear_combinaison(this->theta0, {}, 0.0);

    for (int i = 0; i < n; ++i) {
        auto j = ((this->history_count - 1) % 1000) - i;

        if (j < 0) j += 1000;
        if (j >= 1000) j -= 1000;

        sum_eval += this->history_eval[j];
        sum_theta = Utils::sum(sum_theta, this->history_theta[j]);
    }

    auto alpha = 1.0 / (1.0 * double(n));

    return {
        alpha * sum_eval,
        Utils::linear_combinaison(sum_theta, {}, alpha, 0.0)
    };
};

std::pair<double, std::vector<double>> Minimizer::average_best_evals(int n)
{
    if (n <= 0) n = 1;
    if (n > 1000) n = 1000;
    if (n > this->best_count) n = this->best_count;

    auto sum_eval = 0.0;
    auto sum_theta = Utils::linear_combinaison(this->theta0, {}, 0.0);

    for (int i = 0; i < n; ++i) {
        auto j = ((this->best_count - 1) % 1000) - i;

        if (j < 0) j += 1000;
        if (j >= 1000) j -= 1000;

        sum_eval += this->best_eval[j];
        sum_theta = Utils::sum(sum_theta, this->best_theta[j]);
    }

    auto alpha = 1.0 / (1.0 * double(n));

    return {
        alpha * sum_eval,
        Utils::linear_combinaison(sum_theta, {}, alpha, 0.0)
    };
};

std::vector<double> Minimizer::rprop(std::vector<double> theta, std::vector<double> gradient)
{
    auto previous_g = gradient;

    if (!this->rprop_previous_g.empty()) {
        previous_g = this->rprop_previous_g;
    }

    auto delta = gradient;
    delta = Utils::copy_and_fill(delta, 0.5);

    if (!this->rprop_previous_delta.empty()) {
        delta = this->rprop_previous_delta;
    }

    auto p = Utils::hadamard_product(previous_g, gradient);

    // printf("gradient = %s\n", Utils::pretty(gradient).c_str());
    // printf("old_g    = %s\n", Utils::pretty(previous_g).c_str());
    // printf("p        = %s\n", Utils::pretty(p).c_str());

    std::vector<double> g = {};
    std::vector<double> eta = {};

    for (auto i = 0; i < p.size(); ++i) {
        if (p[i] > 0) eta.push_back(1.1);
        if (p[i] < 0) eta.push_back(0.5);
        if (p[i] == 0) eta.push_back(1.0);

        delta[i] = eta[i] * delta[i];
        delta[i] = std::min(50.0, delta[i]);
        delta[i] = std::max(0.000001, delta[i]);

        g.push_back(gradient[i]);
    }

    // printf("g        = %s\n", Utils::pretty(g).c_str());
    // printf("eta      = %s\n", Utils::pretty(eta).c_str());
    // printf("delta    = %s\n", Utils::pretty(delta).c_str());

    this->rprop_previous_g = g;
    this->rprop_previous_delta = delta;

    auto s = Utils::hadamard_product(delta, Utils::sign(g));

    // printf("sign(g)  = %s\n", Utils::pretty(Utils::sign(g)).c_str());
    // printf("s        = %s\n", Utils::pretty(s).c_str());

    return s;
};

};