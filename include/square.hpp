#pragma once

#include <array>
#include <vector>

#include "file.hpp"
#include "rank.hpp"
#include "operations.hpp"

namespace goldfish {

enum Square {
    A1 = 0  , B1, C1, D1, E1, F1, G1, H1,
    A2 = 16 , B2, C2, D2, E2, F2, G2, H2,
    A3 = 32 , B3, C3, D3, E3, F3, G3, H3,
    A4 = 48 , B4, C4, D4, E4, F4, G4, H4,
    A5 = 64 , B5, C5, D5, E5, F5, G5, H5,
    A6 = 80 , B6, C6, D6, E6, F6, G6, H6,
    A7 = 96 , B7, C7, D7, E7, F7, G7, H7,
    A8 = 112, B8, C8, D8, E8, F8, G8, H8,

    // Non-square != 0 to avoid errors.
    NO_SQUARE = 127,

    // These are our move directions
    // NORTH = north, EAST = east, SOUTH = south, WEST = west
    NORTH = 16, EAST = 1, SOUTH = -16, WEST = -1,
    NORTH_EAST = NORTH + EAST, SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST, NORTH_WEST = NORTH + WEST

};

ENABLE_FULL_OPERATORS_ON(Square);

namespace Squares {

constexpr int MASK = 0x7F;

constexpr int VALUES_LENGTH = 128;
constexpr int VALUES_SIZE = 64;
constexpr std::array<Square, VALUES_SIZE> values{
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8
};


const std::vector<std::vector<Square>> pawn_directions{
    {NORTH, NORTH_EAST, NORTH_WEST}, // Color::WHITE
    {SOUTH, SOUTH_EAST, SOUTH_WEST}  // Color::BLACK
};

const std::vector<Square> knight_directions = {
    NORTH + NORTH + EAST,
    NORTH + NORTH + WEST,
    NORTH + EAST + EAST,
    NORTH + WEST + WEST,
    SOUTH + SOUTH + EAST,
    SOUTH + SOUTH + WEST,
    SOUTH + EAST + EAST,
    SOUTH + WEST + WEST
};

const std::vector<Square> bishop_directions = {
    NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST
};

const std::vector<Square> rook_directions = {
    NORTH, EAST, SOUTH, WEST
};

const std::vector<Square> queen_directions = {
    NORTH, EAST, SOUTH, WEST,
    NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST
};

const std::vector<Square> king_directions = {
    NORTH, EAST, SOUTH, WEST,
    NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST
};

inline constexpr bool is_valid(Square square) {
    return (square & 0x88) == 0;
}

inline constexpr Square value_of(File file, Rank rank) {
    return Square((static_cast<int>(rank) << 4) + static_cast<int>(file));
}

inline constexpr File get_file(Square square) {
    return static_cast<File>(square & 0xF);
}

inline constexpr Rank get_rank(Square square) {
    return static_cast<Rank>(square >> 4);
}

inline constexpr Square invert(Square square) {
    const File f = get_file(square);
    const Rank r = get_rank(square);
    return value_of(f, Ranks::invert(r));
}

}

}
