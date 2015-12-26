#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include "position.h"
#include "bitboards.h"
#include "move.h"
#include "types.h"

using namespace std;

const std::string RANDOM_FEN = "3r1rk1/p3qppp/2bb4/2p5/3p4/1P2P3/PBQN1PPP/2R2RK1 w - - 0 1";

const std::string ENPASSANT_D6 = "rnbqkb1r/ppp1pppp/5n2/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1";

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

  p.clear();
  p = Position(ENPASSANT_D6);
  EXPECT_EQ(SQ_D6, p.enpassantTarget);
  
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


TEST (Position, occupied) {
  Position p = Position(RANDOM_FEN);
  EXPECT_TRUE(p.occupied(SQ_D4));
  EXPECT_TRUE(p.occupied(SQ_E3));
  EXPECT_FALSE(p.occupied(SQ_D4, WHITE));
  ASSERT_TRUE(p.pieces[BLACK][NO_PIECE_TYPE] == p.pieces[WHITE][NO_PIECE_TYPE]);
}



TEST (Position, psudoLegalPawn) {
  Position p = Position(STARTING_FEN);
  Move legal1 = Move(SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_MOVE);
  Move legal2 = Move(SQ_A2, SQ_A3);
  Move illegal1 = Move(SQ_D2, SQ_E5);
  Move illegal2 = Move(SQ_A2, SQ_B3);
  EXPECT_TRUE(p.psudoLegalPawn(legal1));
  EXPECT_TRUE(p.psudoLegalPawn(legal2));
  EXPECT_FALSE(p.psudoLegalPawn(illegal1));
  EXPECT_FALSE(p.psudoLegalPawn(illegal2));

  p.clear();
  p = Position(ENPASSANT_D6);
  Move legal_ep = Move(SQ_E5, SQ_D6, ENPASSANT_CAPTURE_MOVE);
  EXPECT_TRUE(p.psudoLegalPawn(legal_ep)) << "p.enpassantTarget = "
					  << p.enpassantTarget;
}


TEST (Position, ownKingInCheckAfterMove) {
  /* in this position, it's black to move. Pawn on g7 is
   * pinned to the king by a rook on g3. So gxf6 is not allowed
   */
  Position p = Position("3r1rk1/p3qppp/2bb1P2/2p5/3p4/1P2P1R1/PBQN2PP/2R3K1 b KQkq - 0 1");
  Move illegal = Move(SQ_G7, SQ_F6, CAPTURE_MOVE);
  ASSERT_TRUE(p.ownKingInCheckAfterMove(illegal));
  p = Position(ENPASSANT_D6);
  Move legal = Move(SQ_G1, SQ_F3);
  ASSERT_FALSE(p.ownKingInCheckAfterMove(legal));
}


TEST (Position, legal) {
  Position p = Position(RANDOM_FEN);
  EXPECT_TRUE(p.legal(Move(SQ_E3, SQ_D4)));
  EXPECT_FALSE(p.legal(Move(SQ_E3, SQ_E5)));
  EXPECT_TRUE(p.legal(Move(SQ_C2, SQ_C4)));
  EXPECT_TRUE(p.legal(Move(SQ_C2, SQ_H7)));
  EXPECT_FALSE(p.legal(Move(SQ_B2, SQ_H4)));
  EXPECT_FALSE(p.legal(Move(SQ_B2, SQ_E5)));
}
