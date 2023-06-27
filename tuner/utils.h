#pragma once

#include <iostream>
#include <math.h>
#include <map>
#include <vector>

namespace Utils
{

static double norm2(std::vector<double> m)
{
    double result = 0.0;

    for (auto value : m) {
        result += value * value;
    }

    return std::sqrt(result);
};

static double norm1(std::vector<double> m)
{
    double result = 0.0;

    for (auto value : m) {
        result += std::abs(value);
    }

    return result;
};

static std::vector<double> linear_combinaison(std::vector<double> m1, std::vector<double> m2, double alpha = 1.0, double beta = 0.0)
{
    if (m1.size() != m2.size()) {
        m2 = m1;
        beta = 0.0;
    }

    std::vector<double> m;

    for (auto i = 0; i < m1.size(); ++i) {
        m.push_back(alpha * m1[i] + beta * m2[i]);
    }

    return m;
};

static std::vector<double> difference(std::vector<double> m1, std::vector<double> m2)
{
    return Utils::linear_combinaison(m1, m2, 1.0, -1.0);
};

static std::vector<double> sum(std::vector<double> m1, std::vector<double> m2)
{
    return Utils::linear_combinaison(m1, m2, 1.0, 1.0);
};

static std::vector<double> hadamard_product(std::vector<double> m1, std::vector<double> m2)
{
    if (m1.size() != m2.size()) {
        for (auto i = 0; i < m1.size(); ++i) {
            m2.push_back(0.0);
        }
    }

    std::vector<double> m;

    for (auto i = 0; i < m1.size(); ++i) {
        m.push_back(m1[i] * m2[i]);
    }

    return m;
};

static double regulizer(std::vector<double> m, double lambd, double alpha)
{
    return lambd * ((1 - alpha) * Utils::norm1(m) + alpha * Utils::norm2(m));
};

template <typename T>
static T sign_of(T x)
{
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
};

static std::vector<double> sign(std::vector<double> m)
{
    for (auto i = 0; i < m.size(); ++i) {
        m[i] = Utils::sign_of(m[i]);
    }

    return m;
};

static std::vector<double> sqrt(std::vector<double> m)
{
    for (auto i = 0; i < m.size(); ++i) {
        m[i] = std::sqrt(m[i]);
    }

    return m;
};

static std::vector<double> copy_and_fill(std::vector<double> m, double value)
{
    for (auto i = 0; i < m.size(); ++i) {
        m[i] = value;
    }

    return m;
};

static std::string pretty(std::vector<double> m)
{
    std::string result = std::string("{ ");

    for (auto i = 0; i < m.size(); ++i) {
        result.append(std::to_string(m[i]));

        if (i < m.size() - 1) {
            result.append(",");
        }

        result.append(" ");
    }

    result.append("}");

    return result;
};

}