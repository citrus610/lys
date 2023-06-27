#include "score.h"
#include "spsa.h"

int main()
{
    auto result = Spsa::Minimizer(
        get_score,
        weight_to_theta(Eval::DEFAULT_WEIGHT),
        10000,
        constrain
    ).run();

    std::ofstream o("result.txt", std::ofstream::out | std::ofstream::trunc);
    o << Utils::pretty(result);
    o.close();

    return 0;
};