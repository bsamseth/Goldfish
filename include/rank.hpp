#pragma once

#include <array>
#include <cassert>
#include <iostream>

#include "operations.hpp"

namespace goldfish {

enum Rank {
    R1, R2, R3, R4, R5, R6, R7, R8, NO_RANK
};

ENABLE_BASE_OPERATORS_ON(Rank);
ENABLE_INCR_OPERATORS_ON(Rank);

namespace Ranks {

constexpr int VALUES_SIZE = 8;

constexpr std::array<Rank, VALUES_SIZE> values = {
    Rank::R1, Rank::R2, Rank::R3, Rank::R4, Rank::R5, Rank::R6, Rank::R7, Rank::R8
};

inline constexpr bool is_valid(Rank rank) {
    return rank != Rank::NO_RANK;
}

inline constexpr Rank invert(Rank r) {
    assert(r != Rank::NO_RANK);
    return Rank::R8 - r;
}

}

}

