#pragma once

#include <array>
#include "piecetype.hpp"
#include "color.hpp"

namespace goldfish {

enum Piece {
    NO_PIECE,
    WHITE_PAWN = 1, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN = 9, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
    PIECE_NB = 16
};

constexpr Piece operator~(Piece pc) {
  return Piece(pc ^ 8); // Swap color of piece BLACK_KNIGHT -> WHITE_KNIGHT
}

namespace Pieces {

constexpr int VALUES_SIZE = 12;

constexpr std::array<Piece, VALUES_SIZE> values = {
        WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
        BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING
};

constexpr bool is_valid(Piece pc) {
    return (WHITE_PAWN <= pc && pc <= WHITE_KING) || (BLACK_PAWN <= pc && pc <= BLACK_KING);
}

constexpr Piece value_of(Color c, PieceType pt) {
    return Piece((c << 3) + pt);
}

constexpr PieceType type_of(Piece pc) {
    return PieceType(pc & 7);
}

constexpr Color color_of(Piece pc) {
    assert(pc != NO_PIECE);
    return Color(pc >> 3);
}


}
}
