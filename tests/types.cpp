#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include "types.h"

using namespace std;

TEST(Types, colorSwap) {
  EXPECT_EQ(colorSwap(WHITE), BLACK);
  EXPECT_EQ(colorSwap(BLACK), WHITE);
  EXPECT_EQ(colorSwap(NO_COLOR), NO_COLOR);
}


TEST(Types, makeColor) {
  EXPECT_EQ(BLACK, makeColor(B_KNIGHT));
  EXPECT_EQ(BLACK, makeColor(B_PAWN));
  EXPECT_EQ(BLACK, makeColor(B_KING));
  EXPECT_EQ(WHITE, makeColor(W_PAWN));
  EXPECT_EQ(WHITE, makeColor(W_KING));
  EXPECT_EQ(NO_COLOR, makeColor(NO_PIECE));
}

TEST(Types, makePieceType) {
  EXPECT_EQ(KING, makePieceType(W_KING));
  EXPECT_EQ(PAWN, makePieceType(W_PAWN));
  EXPECT_EQ(KING, makePieceType(B_KING));
  EXPECT_EQ(BISHOP, makePieceType(B_BISHOP));
  EXPECT_EQ(NO_PIECE_TYPE, makePieceType(NO_PIECE));
}

TEST(Types, makePiece) {
  EXPECT_EQ(W_PAWN, makePiece(WHITE, PAWN));
  EXPECT_EQ(B_PAWN, makePiece(BLACK, PAWN));
  EXPECT_EQ(B_KING, makePiece(BLACK, KING));
  EXPECT_EQ(W_KING, makePiece(WHITE, KING));
  EXPECT_EQ(B_BISHOP, makePiece(BLACK, BISHOP));
  EXPECT_EQ(NO_PIECE, makePiece(NO_COLOR, NO_PIECE_TYPE));
  EXPECT_EQ(NO_PIECE, makePiece(NO_COLOR, KING));
  EXPECT_EQ(NO_PIECE, makePiece(WHITE, NO_PIECE_TYPE));
}

TEST(Types, makePieceTypeFromChar) {
  EXPECT_EQ(BISHOP, makePieceTypeFromChar('B'));
  EXPECT_EQ(NO_PIECE_TYPE, makePieceTypeFromChar('X'));
}

TEST(Types, makeColorFromChar) {
  EXPECT_EQ(WHITE, makeColorFromChar('K'));
  EXPECT_EQ(BLACK, makeColorFromChar('n'));
}

TEST(Types, squareArithmetic) {
  EXPECT_EQ(SQ_A2, SQ_A1 + D_NORTH);
  EXPECT_EQ(SQ_G4, SQ_H5 + D_SOUTH_WEST);
}

TEST(Types, makeFile) {
  EXPECT_EQ(FILE_G, makeFile('G'));
  EXPECT_DEATH(makeFile('X'), "");
}

TEST(Types, makeRank) {
  EXPECT_EQ(RANK_5, makeRank('5'));
  EXPECT_EQ(RANK_8, makeRank('8'));
}

TEST(Types, squareName) {
  EXPECT_EQ("h8", SquareName[SQ_H8]);
}

TEST(Types, makeFileFromSquare) {
    EXPECT_EQ(FILE_C, makeFile(SQ_C6));
    EXPECT_EQ(FILE_H, makeFile(SQ_H1));
    EXPECT_EQ(FILE_A, makeFile(SQ_A8));
}

TEST(Types, makeRankFromSquare) {
    EXPECT_EQ(RANK_6, makeRank(SQ_C6));
    EXPECT_EQ(RANK_1, makeRank(SQ_H1));
    EXPECT_EQ(RANK_8, makeRank(SQ_A8));
}

TEST(Types, flipPerspective) {
    EXPECT_EQ(SQ_C3, flipPerspective(SQ_C6));
    EXPECT_EQ(SQ_C6, flipPerspective(SQ_C3));
    EXPECT_EQ(SQ_H1, flipPerspective(SQ_H8));
    EXPECT_EQ(SQ_A1, flipPerspective(SQ_A8));
    EXPECT_EQ(SQ_H8, flipPerspective(SQ_H1));
    EXPECT_EQ(SQ_A8, flipPerspective(SQ_A1));
}
