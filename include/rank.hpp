#pragma once

#include <array>
#include "operations.hpp"
#include "color.hpp"

namespace goldfish {


enum Rank : int {
  RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NB
};

ENABLE_INCR_OPERATORS_ON(Rank)

namespace Ranks {

constexpr int VALUES_SIZE = 8;
constexpr std::array<Rank, VALUES_SIZE> values = {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,
};

constexpr bool is_valid(Rank rank) {
    return RANK_1 <= rank && rank <= RANK_8;
}

constexpr Rank relative_rank(Color c, Rank r) {
    return Rank(r ^ (c * 7));
}

}
}



