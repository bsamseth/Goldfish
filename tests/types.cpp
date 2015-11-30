// Bring in my package's API, which is what I'm testing
#include "types.h"
// Bring in gtest
#include <gtest/gtest.h>

TEST(Types, colorSwap) {
  EXPECT_EQ(colorSwap(WHITE), BLACK);
  EXPECT_EQ(colorSwap(BLACK), WHITE);
  EXPECT_EQ(colorSwap(NO_COLOR), NO_COLOR);
}


TEST(Types, makeColor) {
  EXPECT_EQ(makeColor(B_KNIGHT), BLACK);
  EXPECT_EQ(makeColor(B_PAWN), BLACK);
  EXPECT_EQ(makeColor(B_KING), BLACK);
  EXPECT_EQ(makeColor(W_PAWN), WHITE);
  EXPECT_EQ(makeColor(W_KING), WHITE);
  EXPECT_EQ(makeColor(NO_PIECE), NO_COLOR);
}

TEST(Types, makePieceType) {
  EXPECT_EQ(makePieceType(W_KING), KING);
  EXPECT_EQ(makePieceType(W_PAWN), PAWN);
  EXPECT_EQ(makePieceType(B_KING), KING);
  EXPECT_EQ(makePieceType(B_BISHOP), BISHOP);
  EXPECT_EQ(makePieceType(NO_PIECE), NO_PIECE_TYPE);
}



