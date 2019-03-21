#pragma once

#include "operations.hpp"

#include <array>

namespace goldfish
{
enum Rank
{
    RANK_1 = 0,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    NO_RANK
};

ENABLE_INCR_OPERATORS_ON(Rank)

namespace Ranks
{
constexpr int                           VALUES_SIZE = 8;
constexpr std::array<Rank, VALUES_SIZE> values      = {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
};

inline constexpr bool is_valid(Rank rank)
{
    return rank != Rank::NO_RANK;
}

}  // namespace Ranks
}  // namespace goldfish
