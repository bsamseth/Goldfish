#include "move.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(movetest, test_creation)
{
    Move move = Moves::value_of(MoveType::PAWN_PROMOTION,
                                Square::A7,
                                Square::B8,
                                Piece::WHITE_PAWN,
                                Piece::BLACK_QUEEN,
                                PieceType::KNIGHT);

    EXPECT_EQ(MoveType::PAWN_PROMOTION, Moves::get_type(move));
    EXPECT_EQ(Square::A7, Moves::get_origin_square(move));
    EXPECT_EQ(Square::B8, Moves::get_target_square(move));
    EXPECT_EQ(Piece::WHITE_PAWN, Moves::get_origin_piece(move));
    EXPECT_EQ(Piece::BLACK_QUEEN, Moves::get_target_piece(move));
    EXPECT_EQ(PieceType::KNIGHT, Moves::get_promotion(move));
}

TEST(movetest, test_promotion)
{
    Move move = Moves::value_of(MoveType::PAWN_PROMOTION,
                                Square::B7,
                                Square::C8,
                                Piece::WHITE_PAWN,
                                Piece::BLACK_QUEEN,
                                PieceType::KNIGHT);

    EXPECT_EQ(PieceType::KNIGHT, Moves::get_promotion(move));
}
