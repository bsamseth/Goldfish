#pragma once

#include <cmath>

#include "depth.hpp"

namespace goldfish::Value {

constexpr int INFINITE = 200000;
constexpr int CHECKMATE = 100000;
constexpr int CHECKMATE_THRESHOLD = CHECKMATE - Depth::MAX_PLY;
constexpr int DRAW = 0;
constexpr int NO_VALUE = 300000;

inline constexpr bool is_checkmate(int value) {
    int absvalue = std::abs(value);
    return absvalue >= CHECKMATE_THRESHOLD && absvalue <= CHECKMATE;
}
}
