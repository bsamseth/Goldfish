#pragma once

#include <array>

namespace goldfish {

class Rank {
public:
    static const int R1 = 0;
    static const int R2 = 1;
    static const int R3 = 2;
    static const int R4 = 3;
    static const int R5 = 4;
    static const int R6 = 5;
    static const int R7 = 6;
    static const int R8 = 7;

    static const int NO_RANK = 8;

    static const int VALUES_SIZE = 8;
    static const std::array<int, VALUES_SIZE> values;

    static bool is_valid(int rank);

    Rank() = delete;

    ~Rank() = delete;
};

}



