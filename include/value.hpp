#pragma once

#include <cstdint>
#include <cstdlib>
#include "depth.hpp"

namespace goldfish {

using Value = int32_t;

namespace Values {

constexpr Value INFINITE = 200000;
constexpr Value CHECKMATE = 100000;
constexpr Value CHECKMATE_THRESHOLD = CHECKMATE - Depths::MAX_PLY;
constexpr Value DRAW = 0;
constexpr Value NOVALUE = 300000;

inline constexpr bool is_checkmate(Value value) {
    Value absvalue = std::abs(value);
    return absvalue >= CHECKMATE_THRESHOLD && absvalue <= CHECKMATE;
}

}
}
