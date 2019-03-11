#include "tb.hpp"
#include "notation.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(TableBase, init) {
    const bool result = tb::initialize("/Users/bendik/drive/div/Goldfish/syzygy");
    ASSERT_TRUE(result);

    ASSERT_LE(5U, tb::MAX_MAN);
}

TEST(TableBase, probe_wdl) {
    ASSERT_LT(0U, tb::MAX_MAN);
    const Position pos_win_Kc2(Notation::to_position("8/8/8/8/5kp1/P7/8/1K1N4 w - - 0 1"));   // Kc2 - mate
    const Position pos_win_Na2(Notation::to_position("8/8/8/5N2/8/p7/8/2NK3k w - - 0 1"));   // Na2 - mate
    const Position pos_draw(Notation::to_position("8/3k4/8/8/8/4B3/4KB2/2B5 w - - 0 1"));   // Draw

    ASSERT_EQ(tb::Outcome::WIN, tb::probe_outcome(pos_win_Kc2));
    ASSERT_EQ(tb::Outcome::WIN, tb::probe_outcome(pos_win_Na2));
    ASSERT_EQ(tb::Outcome::DRAW, tb::probe_outcome(pos_draw));
}
