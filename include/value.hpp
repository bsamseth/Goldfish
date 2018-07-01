#pragma once

#include "depth.hpp"

namespace goldfish {

class Value {
public:
    static const int INFINITE = 200000;
    static const int CHECKMATE = 100000;
    static const int CHECKMATE_THRESHOLD = CHECKMATE - Depth::MAX_PLY;
    static const int DRAW = 0;

    static const int NOVALUE = 300000;

    static bool is_checkmate(int value);

    Value() = delete;

    ~Value() = delete;
};

}
