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


TEST(Position, setFromFEN) {
  Position p = Position();
  p.setFromFEN(STARTING_FEN);
  EXPECT_EQ(W_KING, p.board[SQ_E1]);
  EXPECT_EQ(B_PAWN, p.board[SQ_C7]);
  EXPECT_EQ(NO_PIECE, p.board[SQ_E4]);
  EXPECT_EQ((1ULL << SQ_A8) | (1ULL << SQ_H8), p.pieces[BLACK][ROOK]);
  EXPECT_EQ(WHITE, p.sideToMove);
  EXPECT_EQ(ANY_CASTLING, p.castlingRights);

  EXPECT_EQ(NO_SQUARE, p.enpassantTarget);

  EXPECT_EQ(0, p.halfmoveClock);
  EXPECT_EQ(1, p.fullmoveNumber);
  
}

TEST(Position, representationConsistency) {
  Position p = Position();
  p.setFromFEN(STARTING_FEN);
  Move m1 = Move(SQ_E2, SQ_E4), m2 = Move(SQ_E7, SQ_E5);
  p.doMove(m1);
  p.doMove(m2);

  EXPECT_EQ(NO_PIECE, p.board[SQ_E2]);
  EXPECT_EQ(B_PAWN, p.board[SQ_E5]);

  Bitboard black = p.getBoardForColor(BLACK);
  Bitboard white = p.getBoardForColor(WHITE);
  Bitboard any   = black | white;
  Bitboard noneB = ~black;
  Bitboard noneW = ~white;
  Bitboard none  = ~(noneB ^ noneW);
  EXPECT_EQ(none, ~any);
  EXPECT_EQ(black, ~(white | none));
  EXPECT_EQ(white, ~(black | none));

  EXPECT_EQ(p.pieces[BLACK][NO_PIECE_TYPE], p.pieces[WHITE][NO_PIECE_TYPE]);
}

TEST(Position, doMove) {
  
  Position p = Position();
  p.setFromFEN(STARTING_FEN);
  Move m1 = Move(SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_MOVE);
  p.doMove(m1);
  
  EXPECT_EQ(NO_PIECE, p.board[SQ_E2]);
  EXPECT_EQ(W_PAWN, p.board[SQ_E4]);
  
  EXPECT_EQ(W_PAWN, p.stateInfo->lastMove_originPiece);
  EXPECT_EQ(NO_PIECE, p.stateInfo->lastMove_destinationPiece);
  EXPECT_EQ(NO_SQUARE, p.stateInfo->lastMove_enpassantTarget);

  EXPECT_EQ(BLACK, p.sideToMove);
  EXPECT_EQ(1, p.fullmoveNumber);
  EXPECT_EQ(0, p.halfmoveClock);

  EXPECT_EQ(SQ_E3, p.enpassantTarget);
  EXPECT_EQ(1, p.moveList.size());
}

TEST(Position, undoMove) {
  Position p = Position();
  p.setFromFEN(STARTING_FEN);
  Move m1 = Move(SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_MOVE);
  Move m2 = Move(SQ_E7, SQ_E5, DOUBLE_PAWN_PUSH_MOVE);
  Move m3 = Move(SQ_G1, SQ_F3);
  p.doMove(m1);
  p.doMove(m2);
  p.doMove(m3);
  p.undoMove();

  EXPECT_TRUE(m2.doublePawnPush());

  EXPECT_EQ(NO_PIECE, p.board[SQ_E2]);
  EXPECT_EQ(W_PAWN, p.board[SQ_E4]);
  EXPECT_EQ(B_PAWN, p.board[SQ_E5]);
  EXPECT_EQ(NO_PIECE, p.board[SQ_E7]);
  EXPECT_EQ(W_KNIGHT, p.board[SQ_G1]);
  EXPECT_EQ(NO_PIECE, p.board[SQ_F3]);
  
  EXPECT_EQ(W_PAWN, p.stateInfo->lastMove_originPiece);
  EXPECT_EQ(NO_PIECE, p.stateInfo->lastMove_destinationPiece);
  EXPECT_EQ(1, p.stateInfo->previous_fullmoveNumber);

  EXPECT_EQ(WHITE, p.sideToMove);
  EXPECT_EQ(2, p.fullmoveNumber);
  EXPECT_EQ(0, p.halfmoveClock);

  EXPECT_EQ(SQ_E6, p.enpassantTarget);
  EXPECT_EQ(2, p.moveList.size());
}
