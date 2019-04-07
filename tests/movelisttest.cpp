#include "movelist.hpp"

#include "move.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(movelisttest, test)
{
    MoveList<MoveEntry> move_list;

    Move m1 = Move::NO_MOVE;
    Move m2 = Moves::value_of(MoveType::NORMAL,
                              Square::A2,
                              Square::A3,
                              Piece::WHITE_PAWN,
                              Piece::NO_PIECE,
                              PieceType::NO_PIECE_TYPE);
    Move m3 = Moves::value_of(MoveType::NORMAL,
                              Square::A3,
                              Square::A4,
                              Piece::WHITE_PAWN,
                              Piece::NO_PIECE,
                              PieceType::NO_PIECE_TYPE);
    Move m4 = Moves::value_of(MoveType::NORMAL,
                              Square::A4,
                              Square::A5,
                              Piece::WHITE_PAWN,
                              Piece::NO_PIECE,
                              PieceType::NO_PIECE_TYPE);

    ASSERT_EQ(0, move_list.size);
    move_list.entries[move_list.size++].move = m1;
    ASSERT_EQ(1, move_list.size);
    ASSERT_EQ(m1, move_list.entries[0].move);
    move_list.entries[move_list.size++].move = m2;
    move_list.entries[move_list.size++].move = m3;
    move_list.entries[move_list.size++].move = m4;
    ASSERT_EQ(4, move_list.size);
    move_list.add_killer(m3);
    ASSERT_EQ(m3, move_list.entries[0].move);
    ASSERT_EQ(m1, move_list.entries[1].move);
    ASSERT_EQ(m2, move_list.entries[2].move);
    ASSERT_EQ(m4, move_list.entries[3].move);
}
