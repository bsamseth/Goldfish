#include "notation.hpp"
#include "file.hpp"
#include "rank.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(notationtest, test_standard_position) {
    Position position(Notation::to_position(Notation::STANDARDPOSITION));

    // Test pawns
    for ( auto file : Files::values) {
        EXPECT_EQ(Piece::WHITE_PAWN, position.board[static_cast<int>(Squares::value_of(file, Rank::R2))]);
        EXPECT_EQ(Piece::BLACK_PAWN, position.board[static_cast<int>(Squares::value_of(file, Rank::R7))]);
    }

    // Test knights
    EXPECT_EQ(Piece::WHITE_KNIGHT, position.board[static_cast<int>(Square::B1)]);
    EXPECT_EQ(Piece::WHITE_KNIGHT, position.board[static_cast<int>(Square::G1)]);
    EXPECT_EQ(Piece::BLACK_KNIGHT, position.board[static_cast<int>(Square::B8)]);
    EXPECT_EQ(Piece::BLACK_KNIGHT, position.board[static_cast<int>(Square::G8)]);

    // Test bishops
    EXPECT_EQ(Piece::WHITE_BISHOP, position.board[static_cast<int>(Square::C1)]);
    EXPECT_EQ(Piece::WHITE_BISHOP, position.board[static_cast<int>(Square::F1)]);
    EXPECT_EQ(Piece::BLACK_BISHOP, position.board[static_cast<int>(Square::C8)]);
    EXPECT_EQ(Piece::BLACK_BISHOP, position.board[static_cast<int>(Square::F8)]);

    // Test rooks
    EXPECT_EQ(Piece::WHITE_ROOK, position.board[static_cast<int>(Square::A1)]);
    EXPECT_EQ(Piece::WHITE_ROOK, position.board[static_cast<int>(Square::H1)]);
    EXPECT_EQ(Piece::BLACK_ROOK, position.board[static_cast<int>(Square::A8)]);
    EXPECT_EQ(Piece::BLACK_ROOK, position.board[static_cast<int>(Square::H8)]);

    // Test queens
    EXPECT_EQ(Piece::WHITE_QUEEN, position.board[static_cast<int>(Square::D1)]);
    EXPECT_EQ(Piece::BLACK_QUEEN, position.board[static_cast<int>(Square::D8)]);

    // Test kings
    EXPECT_EQ(Piece::WHITE_KING, position.board[static_cast<int>(Square::E1)]);
    EXPECT_EQ(Piece::BLACK_KING, position.board[static_cast<int>(Square::E8)]);

    EXPECT_EQ(8 * PieceTypes::PAWN_VALUE
                  + 2 * PieceTypes::KNIGHT_VALUE
                  + 2 * PieceTypes::BISHOP_VALUE
                  + 2 * PieceTypes::ROOK_VALUE
                  + PieceTypes::QUEEN_VALUE
                  + PieceTypes::KING_VALUE,
              position.material[static_cast<int>(Color::WHITE)]);
    EXPECT_EQ(8 * PieceTypes::PAWN_VALUE
                  + 2 * PieceTypes::KNIGHT_VALUE
                  + 2 * PieceTypes::BISHOP_VALUE
                  + 2 * PieceTypes::ROOK_VALUE
                  + PieceTypes::QUEEN_VALUE
                  + PieceTypes::KING_VALUE,
              position.material[static_cast<int>(Color::BLACK)]);

    // Test castling
    EXPECT_NE(Castling::NO_CASTLING, position.castling_rights &Castling::WHITE_KING_SIDE);
    EXPECT_NE(Castling::NO_CASTLING, position.castling_rights &Castling::WHITE_QUEEN_SIDE);
    EXPECT_NE(Castling::NO_CASTLING, position.castling_rights &Castling::BLACK_KING_SIDE);
    EXPECT_NE(Castling::NO_CASTLING, position.castling_rights &Castling::BLACK_QUEEN_SIDE);

    // Test en passant
    EXPECT_EQ(Square::NO_SQUARE, position.enpassant_square);

    // Test active color
    EXPECT_EQ(Color::WHITE, position.active_color);

    // Test half move clock
    EXPECT_EQ(0, position.halfmove_clock);

    // Test full move number
    EXPECT_EQ(1, position.get_fullmove_number());
}
