#pragma once

#include <array>

namespace goldfish {

enum class Rank {
    R1, R2, R3, R4, R5, R6, R7, R8, NO_RANK
};

inline constexpr Rank& operator++ (Rank& rank) {
    return rank = static_cast<Rank>(static_cast<int>(rank) + 1);
}

inline constexpr Rank& operator-- (Rank& rank) {
    return rank = static_cast<Rank>(static_cast<int>(rank) - 1);
}

inline constexpr Rank& operator+= (Rank& rank, int increment) {
    int rank_n = static_cast<int>(rank) + increment;

    if (rank_n >= static_cast<int>(Rank::NO_RANK))
        return rank = Rank::NO_RANK;

    return rank = static_cast<Rank>(rank_n);
}

namespace Ranks {

constexpr int VALUES_SIZE = 8;

constexpr std::array<Rank, VALUES_SIZE> values = {
    Rank::R1, Rank::R2, Rank::R3, Rank::R4, Rank::R5, Rank::R6, Rank::R7, Rank::R8
};

constexpr bool is_valid(Rank rank) {
    return rank != Rank::NO_RANK;
}

}

}

