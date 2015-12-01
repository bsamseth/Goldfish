#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include "position.h"
#include "bitboards.h"
#include "move.h"
#include "types.h"

using namespace std;

TEST(Position, clear) {
  Position p = Position();
  p.clear();
  for (int c = WHITE; c < NUMBER_OF_COLORS - 1; c++) {
    for (int pt = PAWN; pt < NUMBER_OF_PIECE_TYPES; pt++) {
      EXPECT_FALSE(p.pieces[c][pt] & BITBOARD_UNIVERSE);
    }
    EXPECT_TRUE(p.pieces[c][NO_PIECE_TYPE] & BITBOARD_UNIVERSE);
  }
  for (int sq = SQ_A1; sq < NUMBER_OF_SQUARES; sq++) {
    EXPECT_EQ(NO_PIECE, p.board[sq]);
  }

}

TEST(Position, putPiece) {
  Position p = Position();
  p.clear();
  p.putPiece(SQ_A5, W_KING);
  EXPECT_EQ(W_KING, p.board[SQ_A5]);
  EXPECT_TRUE(p.pieces[WHITE][KING] == (1ULL << SQ_A5));
  EXPECT_TRUE(p.pieces[WHITE][NO_PIECE_TYPE] == ~(1ULL << SQ_A5));
  EXPECT_TRUE(p.pieces[BLACK][NO_PIECE_TYPE] == ~(1ULL << SQ_A5));
}

TEST(Position, putNoPiece) {
  Position p = Position();
  p.clear();
  p.putPiece(SQ_A5, W_KING); // fill with previous values
  p.putPiece(SQ_A5, B_BISHOP);
  p.putPiece(SQ_A5, NO_PIECE);
  EXPECT_EQ(NO_PIECE, p.board[SQ_A5]);
  EXPECT_TRUE(p.pieces[WHITE][NO_PIECE_TYPE] & (1ULL << SQ_A5));
  EXPECT_TRUE(p.pieces[BLACK][NO_PIECE_TYPE] & (1ULL << SQ_A5));
  for (int pt = PAWN; pt < NUMBER_OF_PIECE_TYPES; pt++) {
    EXPECT_TRUE(p.pieces[WHITE][pt] == BITBOARD_EMPTY);
    EXPECT_TRUE(p.pieces[BLACK][pt] == BITBOARD_EMPTY);
  }
}
