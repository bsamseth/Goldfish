#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include "move.h"
#include "types.h"

using namespace std;

TEST(Move, getFromTo) {
  Move m = Move(SQ_G5, SQ_H8);
  EXPECT_EQ(SQ_G5, m.getFrom());
  EXPECT_EQ(SQ_H8, m.getTo());
}

TEST(Move, setFromTo) {
  Move m = Move(SQ_E1, SQ_A6);
  m.setFrom(SQ_A1);
  m.setTo(SQ_H3);
  EXPECT_EQ(SQ_A1, m.getFrom());
  EXPECT_EQ(SQ_H3, m.getTo());
}

TEST(Move, getFlag) {
  Move m = Move(SQ_E1, SQ_G1, QUEEN_PROMO_CAPTURE_MOVE);
  EXPECT_EQ(QUEEN_PROMO_CAPTURE_MOVE, m.getFlag());
}

TEST(Move, str) {
  Move m = Move(SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_MOVE);
  EXPECT_EQ("e2e4", m.str());
}

TEST(Move, capture) {
    Move m = Move(SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_MOVE);
    Move m2 = Move(SQ_G7, SQ_F8, QUEEN_PROMO_CAPTURE_MOVE);
    EXPECT_FALSE(m.capture());
    EXPECT_TRUE(m2.capture());
}
TEST(Move, promotion) {
  Move m2 = Move(SQ_G7, SQ_F8, QUEEN_PROMO_CAPTURE_MOVE);
  EXPECT_TRUE(m2.promotion());
}
