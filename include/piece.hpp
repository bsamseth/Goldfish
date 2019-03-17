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


// Move directions
constexpr std::array<std::array<Direction, 3>, 2> pawn_directions = {
    {NORTH, NORTH_EAST, NORTH_WEST}, // Color::WHITE
    {SOUTH, SOUTH_EAST, SOUTH_WEST}  // Color::BLACK
};

constexpr std::array<Direction, 8> knight_directions = {
    NORTH + NORTH + EAST,
    NORTH + NORTH + WEST,
    NORTH + EAST + EAST,
    NORTH + WEST + WEST,
    SOUTH + SOUTH + EAST,
    SOUTH + SOUTH + WEST,
    SOUTH + EAST + EAST,
    SOUTH + WEST + WEST
};
constexpr std::array<Direction, 4> bishop_directions = {
    NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST
};
constexpr std::array<Direction, 4> rook_directions = {
    NORTH, EAST, SOUTH, WEST
};
constexpr std::array<Direction, 8> queen_directions = {
    NORTH, EAST, SOUTH, WEST,
    NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST
};
constexpr std::array<Direction, 8> king_directions = queen_directions;

template<Piece pc>
constexpr auto& directions(Piece pc) {
    if constexpr (type_of(pc) == PieceType::PAWN) {
        return pawn_directions[color_of(pc)];
    }
}




}
}
