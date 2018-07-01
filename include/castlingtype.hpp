#pragma once

#include <array>

namespace goldfish {

class CastlingType {
public:
    static const int KING_SIDE = 0;
    static const int QUEEN_SIDE = 1;

    static const int NO_CASTLING_TYPE = 2;

    static const int VALUES_SIZE = 2;
    static const std::array<int, VALUES_SIZE> values;

private:
    CastlingType();

    ~CastlingType();
};

}

