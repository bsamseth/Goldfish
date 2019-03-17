#pragma once

#include "file.hpp"
#include "operations.hpp"
#include "rank.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace goldfish
{
enum Square : int
{
    SQ_A1,
    SQ_B1,
    SQ_C1,
    SQ_D1,
    SQ_E1,
    SQ_F1,
    SQ_G1,
    SQ_H1,
    SQ_A2,
    SQ_B2,
    SQ_C2,
    SQ_D2,
    SQ_E2,
    SQ_F2,
    SQ_G2,
    SQ_H2,
    SQ_A3,
    SQ_B3,
    SQ_C3,
    SQ_D3,
    SQ_E3,
    SQ_F3,
    SQ_G3,
    SQ_H3,
    SQ_A4,
    SQ_B4,
    SQ_C4,
    SQ_D4,
    SQ_E4,
    SQ_F4,
    SQ_G4,
    SQ_H4,
    SQ_A5,
    SQ_B5,
    SQ_C5,
    SQ_D5,
    SQ_E5,
    SQ_F5,
    SQ_G5,
    SQ_H5,
    SQ_A6,
    SQ_B6,
    SQ_C6,
    SQ_D6,
    SQ_E6,
    SQ_F6,
    SQ_G6,
    SQ_H6,
    SQ_A7,
    SQ_B7,
    SQ_C7,
    SQ_D7,
    SQ_E7,
    SQ_F7,
    SQ_G7,
    SQ_H7,
    SQ_A8,
    SQ_B8,
    SQ_C8,
    SQ_D8,
    SQ_E8,
    SQ_F8,
    SQ_G8,
    SQ_H8,
    SQ_NONE,

    SQUARE_NB = 64
};

enum Direction : int
{
    NORTH = 8,
    EAST  = 1,
    SOUTH = -NORTH,
    WEST  = -EAST,

    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST
};

ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_FULL_OPERATORS_ON(Direction)

constexpr Square operator~(Square s)
{
    return Square(s ^ SQ_A8);  // Vertical flip SQ_A1 -> SQ_A8
}

constexpr Square operator+(Square s, Direction d)
{
    return Square(int(s) + int(d));
}
constexpr Square operator-(Square s, Direction d)
{
    return Square(int(s) - int(d));
}
inline Square& operator+=(Square& s, Direction d)
{
    return s = s + d;
}
inline Square& operator-=(Square& s, Direction d)
{
    return s = s - d;
}

namespace Squares
{
constexpr int VALUES_SIZE = Square::SQUARE_NB;

constexpr std::array<Square, VALUES_SIZE> values
    = {SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1, SQ_A2, SQ_B2, SQ_C2,
       SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2, SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3,
       SQ_G3, SQ_H3, SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4, SQ_A5,
       SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5, SQ_A6, SQ_B6, SQ_C6, SQ_D6,
       SQ_E6, SQ_F6, SQ_G6, SQ_H6, SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7,
       SQ_H7, SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8};

const std::vector<std::vector<Direction>> pawn_directions = {
    {NORTH, NORTH_EAST, NORTH_WEST},  // Color::WHITE
    {SOUTH, SOUTH_EAST, SOUTH_WEST}   // Color::BLACK
};

const std::vector<Direction> knight_directions = {NORTH + NORTH + EAST,
                                                  NORTH + NORTH + WEST,
                                                  NORTH + EAST + EAST,
                                                  NORTH + WEST + WEST,
                                                  SOUTH + SOUTH + EAST,
                                                  SOUTH + SOUTH + WEST,
                                                  SOUTH + EAST + EAST,
                                                  SOUTH + WEST + WEST};
const std::vector<Direction> bishop_directions
    = {NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};
const std::vector<Direction> rook_directions = {NORTH, EAST, SOUTH, WEST};
const std::vector<Direction> queen_directions
    = {NORTH, EAST, SOUTH, WEST, NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};
const std::vector<Direction> king_directions
    = {NORTH, EAST, SOUTH, WEST, NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};

constexpr bool is_valid(Square s)
{
    return s >= SQ_A1 && s <= SQ_H8;
}

constexpr Square value_of(File f, Rank r)
{
    return Square((r << 3) + f);
}

constexpr File file_of(Square s)
{
    return File(s & 7);
}

constexpr Rank rank_of(Square s)
{
    return Rank(s >> 3);
}

constexpr Square relative_square(Color c, Square s)
{
    return Square(s ^ (c * 56));
}

}  // namespace Squares

// This function has to be defined here, but belongs logically in goldfish::Ranks.
namespace Ranks
{
constexpr Rank relative_rank(Color c, Square s)
{
    return relative_rank(c, Squares::rank_of(s));
}
}  // namespace Ranks

}  // namespace goldfish
