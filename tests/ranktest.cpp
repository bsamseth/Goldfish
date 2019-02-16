#include "rank.hpp"
#include "bitboard.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(ranktest, test_values) {
    for ( auto rank : Ranks::values) {
        EXPECT_EQ(rank, Ranks::values[rank]);
    }
}

TEST(ranktest, test_range) {
    using namespace Bitboard;
    ASSERT_EQ(Rank6BB | Rank7BB | Rank8BB, Ranks::range(Rank::RANK_6));
    ASSERT_EQ(Rank5BB | Rank6BB | Rank7BB , Ranks::range(Rank::RANK_5, Rank::RANK_8));
}
