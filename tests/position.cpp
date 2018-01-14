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

const std::string POSSIBLE_CASTLE = "r2qkbnr/ppp1pppp/2n5/3p4/6b1/5NP1/PPPPPPBP/RNBQK2R w KQkq - 4 4";

TEST(Position, clear) {
  Position p = Position();
  cout << "DONE with setup" << endl;
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

  EXPECT_EQ(B_PAWN, p.stateInfo->lastMove_originPiece);
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

TEST (Position_castle, doMove) {
  Position p = Position("r3k3/8/8/2B5/2b5/8/8/R3K2R w KQq - 0 1");
  p.doMove(Move(SQ_E1, SQ_C1, QUEEN_CASTLE_MOVE));

  EXPECT_EQ(W_KING, p.board[SQ_C1]);
  EXPECT_EQ(W_ROOK, p.board[SQ_D1]);
  EXPECT_EQ(BLACK_OOO, p.castlingRights);
  EXPECT_EQ(SQ_D1, p.kingpassantTarget);
}

TEST (Position_castle, undoMove) {
  Position p = Position("4k3/8/8/2B5/2b5/8/8/R3K2R w KQ - 0 1");
  p.doMove(Move(SQ_E1, SQ_C1, QUEEN_CASTLE_MOVE));
  p.undoMove();

  EXPECT_EQ(W_KING, p.board[SQ_E1]);
  EXPECT_EQ(W_ROOK, p.board[SQ_A1]);
  EXPECT_EQ(NO_PIECE, p.board[SQ_D1]);
  EXPECT_EQ(NO_PIECE, p.board[SQ_C1]);
  EXPECT_EQ(NO_PIECE, p.board[SQ_B1]);
  EXPECT_EQ(NO_SQUARE, p.kingpassantTarget);
  EXPECT_EQ(WHITE_OO | WHITE_OOO, p.castlingRights);
}

TEST (Position_score, startpos) {
  Position p = Position(STARTING_FEN);
  int eval = p.score();
  EXPECT_EQ(0, eval);
}

TEST (Position_score, after_e2e4) {
  Position p = Position(STARTING_FEN);
  p.doMove(Move(SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_MOVE));
  int eval = p.score();
  EXPECT_EQ(40, eval);
}

TEST (Zobrist, same_hash_for_different_objects) {
    Move m = Move(SQ_A2, SQ_A4, DOUBLE_PAWN_PUSH_MOVE);
    Position p1 = Position(STARTING_FEN);
    Position p2 = Position(STARTING_FEN);
    p1.doMove(m);
    p2.doMove(m);
    EXPECT_EQ(p1.hash, p2.hash);
}

TEST (Zobrist, undo_restores_hash) {
    Position p = Position(STARTING_FEN);
    Key start_hash = p.hash;
    p.doMove(Move(SQ_A2, SQ_A3));
    p.doMove(Move(SQ_E7, SQ_E5, DOUBLE_PAWN_PUSH_MOVE));
    p.undoMove();
    p.undoMove();
    EXPECT_EQ(start_hash, p.hash);
}

TEST (Zobrist, transposition_gives_same_hash) {
    Move m[6] = { Move(SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_MOVE), Move(SQ_E7, SQ_E5, DOUBLE_PAWN_PUSH_MOVE),
                  Move(SQ_B1, SQ_C3), Move(SQ_G8, SQ_F6), Move(SQ_A2, SQ_A3), Move(SQ_A7, SQ_A6)};
    Position p1 = Position(STARTING_FEN);
    Position p2 = Position(STARTING_FEN);
    p1.doMove(m[0]); p1.doMove(m[1]); p1.doMove(m[2]); p1.doMove(m[3]);
    p2.doMove(m[2]); p2.doMove(m[3]); p2.doMove(m[0]); p2.doMove(m[1]);
    EXPECT_FALSE(p1.hash == p2.hash);  // Different due to enpassant.
    p1.doMove(m[4]); p1.doMove(m[5]);
    p2.doMove(m[4]); p2.doMove(m[5]);
    EXPECT_EQ(p1.hash, p2.hash);      // Equal after all moves.
    // Check that undo works, regardless of move order.
    for (int i = 0; i < 6; i++) {
        p1.undoMove();
        p2.undoMove();
    }
    EXPECT_EQ(p1.hash, p2.hash);
    EXPECT_EQ(p1.hash, Position(STARTING_FEN).hash);
}

