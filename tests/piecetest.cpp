#include "piece.hpp"

#include "color.hpp"
#include "piecetype.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(piecetest, test_values)
{
    for (auto piece : Pieces::values)
    {
        EXPECT_EQ(piece, Pieces::values[piece]);
    }
}

TEST(piecetest, test_value_of)
{
    EXPECT_EQ(Piece::WHITE_PAWN, Pieces::value_of(Color::WHITE, PieceType::PAWN));
    EXPECT_EQ(Piece::WHITE_KNIGHT, Pieces::value_of(Color::WHITE, PieceType::KNIGHT));
    EXPECT_EQ(Piece::WHITE_BISHOP, Pieces::value_of(Color::WHITE, PieceType::BISHOP));
    EXPECT_EQ(Piece::WHITE_ROOK, Pieces::value_of(Color::WHITE, PieceType::ROOK));
    EXPECT_EQ(Piece::WHITE_QUEEN, Pieces::value_of(Color::WHITE, PieceType::QUEEN));
    EXPECT_EQ(Piece::WHITE_KING, Pieces::value_of(Color::WHITE, PieceType::KING));
    EXPECT_EQ(Piece::BLACK_PAWN, Pieces::value_of(Color::BLACK, PieceType::PAWN));
    EXPECT_EQ(Piece::BLACK_KNIGHT, Pieces::value_of(Color::BLACK, PieceType::KNIGHT));
    EXPECT_EQ(Piece::BLACK_BISHOP, Pieces::value_of(Color::BLACK, PieceType::BISHOP));
    EXPECT_EQ(Piece::BLACK_ROOK, Pieces::value_of(Color::BLACK, PieceType::ROOK));
    EXPECT_EQ(Piece::BLACK_QUEEN, Pieces::value_of(Color::BLACK, PieceType::QUEEN));
    EXPECT_EQ(Piece::BLACK_KING, Pieces::value_of(Color::BLACK, PieceType::KING));
}

TEST(piecetest, test_get_type)
{
    EXPECT_EQ(PieceType::PAWN, Pieces::get_type(Piece::WHITE_PAWN));
    EXPECT_EQ(PieceType::PAWN, Pieces::get_type(Piece::BLACK_PAWN));
    EXPECT_EQ(PieceType::KNIGHT, Pieces::get_type(Piece::WHITE_KNIGHT));
    EXPECT_EQ(PieceType::KNIGHT, Pieces::get_type(Piece::BLACK_KNIGHT));
    EXPECT_EQ(PieceType::BISHOP, Pieces::get_type(Piece::WHITE_BISHOP));
    EXPECT_EQ(PieceType::BISHOP, Pieces::get_type(Piece::BLACK_BISHOP));
    EXPECT_EQ(PieceType::ROOK, Pieces::get_type(Piece::WHITE_ROOK));
    EXPECT_EQ(PieceType::ROOK, Pieces::get_type(Piece::BLACK_ROOK));
    EXPECT_EQ(PieceType::QUEEN, Pieces::get_type(Piece::WHITE_QUEEN));
    EXPECT_EQ(PieceType::QUEEN, Pieces::get_type(Piece::BLACK_QUEEN));
    EXPECT_EQ(PieceType::KING, Pieces::get_type(Piece::WHITE_KING));
    EXPECT_EQ(PieceType::KING, Pieces::get_type(Piece::BLACK_KING));
}

TEST(piecetest, test_get_color)
{
    EXPECT_EQ(Color::WHITE, Pieces::get_color(Piece::WHITE_PAWN));
    EXPECT_EQ(Color::BLACK, Pieces::get_color(Piece::BLACK_PAWN));
    EXPECT_EQ(Color::WHITE, Pieces::get_color(Piece::WHITE_KNIGHT));
    EXPECT_EQ(Color::BLACK, Pieces::get_color(Piece::BLACK_KNIGHT));
    EXPECT_EQ(Color::WHITE, Pieces::get_color(Piece::WHITE_BISHOP));
    EXPECT_EQ(Color::BLACK, Pieces::get_color(Piece::BLACK_BISHOP));
    EXPECT_EQ(Color::WHITE, Pieces::get_color(Piece::WHITE_ROOK));
    EXPECT_EQ(Color::BLACK, Pieces::get_color(Piece::BLACK_ROOK));
    EXPECT_EQ(Color::WHITE, Pieces::get_color(Piece::WHITE_QUEEN));
    EXPECT_EQ(Color::BLACK, Pieces::get_color(Piece::BLACK_QUEEN));
    EXPECT_EQ(Color::WHITE, Pieces::get_color(Piece::WHITE_KING));
    EXPECT_EQ(Color::BLACK, Pieces::get_color(Piece::BLACK_KING));
}
