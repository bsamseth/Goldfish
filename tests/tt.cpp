#include "tt.hpp"

#include "evaluation.hpp"
#include "notation.hpp"
#include "position.hpp"

#include "gtest/gtest.h"

using namespace goldfish;
using namespace goldfish::tt;

TEST(ttable, test_store_and_probe)
{
    TranspositionTable tt(1);

    Position pos(Notation::to_position(Notation::STANDARDPOSITION));
    uint64_t key   = pos.zobrist_key;
    Value    value = -Value::CHECKMATE;
    Bound    bound = Bound::EXACT;
    Depth    depth = Depth(254);
    Move     move  = Moves::value_of(MoveType::EN_PASSANT,
                                Square::D5,
                                Square::E6,
                                Piece::WHITE_PAWN,
                                Piece::BLACK_PAWN,
                                PieceType::NO_PIECE_TYPE);

    tt.store(pos.zobrist_key, value, bound, depth, move);

    auto stored = tt.probe(key);

    // Check that everything is stored properly.
    ASSERT_NE(nullptr, stored);
    EXPECT_EQ((key >> 32), stored->key());
    EXPECT_EQ(value, stored->value());
    EXPECT_EQ(bound, stored->bound());
    EXPECT_EQ(depth, stored->depth());
    EXPECT_EQ(move, stored->move());

    // Insert entry with lower depth should not do anything.
    tt.store(pos.zobrist_key, value, bound, depth - 1, move);
    stored = tt.probe(key);

    ASSERT_EQ(depth, stored->depth());

    // Storing a new, better entry, but without a move will retain the move from before,
    // but update the rest.
    tt.store(pos.zobrist_key, value + 1, bound, depth + 1, Move::NO_MOVE);
    stored = tt.probe(key);
    ASSERT_EQ(move, stored->move());
    ASSERT_EQ(value + 1, stored->value());
    ASSERT_EQ(depth + 1, stored->depth());
}
