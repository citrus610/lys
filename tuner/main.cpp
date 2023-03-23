#include <iostream>
#include "tuner.h"

int main()
{
    srand(uint32_t(time(NULL)));
    Tuner tuner;
    tuner.start();

    return 0;
};