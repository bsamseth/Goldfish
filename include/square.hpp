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
    A1 = 0,
    A2 = 16,
    A3 = 32,
    A4 = 48,
    A5 = 64,
    A6 = 80,
    A7 = 96,
    A8 = 112,
    B1 = 1,
    B2 = 17,
    B3 = 33,
    B4 = 49,
    B5 = 65,
    B6 = 81,
    B7 = 97,
    B8 = 113,
    C1 = 2,
    C2 = 18,
    C3 = 34,
    C4 = 50,
    C5 = 66,
    C6 = 82,
    C7 = 98,
    C8 = 114,
    D1 = 3,
    D2 = 19,
    D3 = 35,
    D4 = 51,
    D5 = 67,
    D6 = 83,
    D7 = 99,
    D8 = 115,
    E1 = 4,
    E2 = 20,
    E3 = 36,
    E4 = 52,
    E5 = 68,
    E6 = 84,
    E7 = 100,
    E8 = 116,
    F1 = 5,
    F2 = 21,
    F3 = 37,
    F4 = 53,
    F5 = 69,
    F6 = 85,
    F7 = 101,
    F8 = 117,
    G1 = 6,
    G2 = 22,
    G3 = 38,
    G4 = 54,
    G5 = 70,
    G6 = 86,
    G7 = 102,
    G8 = 118,
    H1 = 7,
    H2 = 23,
    H3 = 39,
    H4 = 55,
    H5 = 71,
    H6 = 87,
    H7 = 103,
    H8 = 119,

    NO_SQUARE = 127,
};

enum Direction : int
{
    NORTH      = 16,
    EAST       = 1,
    SOUTH      = -NORTH,
    WEST       = -EAST,
    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST,
};

ENABLE_FULL_OPERATORS_ON(Direction)

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
constexpr int MASK = 0x7F;

constexpr int VALUES_LENGTH = 128;
constexpr int VALUES_SIZE   = 64;

constexpr std::array<Square, VALUES_SIZE> values
    = {A1, B1, C1, D1, E1, F1, G1, H1, A2, B2, C2, D2, E2, F2, G2, H2,
       A3, B3, C3, D3, E3, F3, G3, H3, A4, B4, C4, D4, E4, F4, G4, H4,
       A5, B5, C5, D5, E5, F5, G5, H5, A6, B6, C6, D6, E6, F6, G6, H6,
       A7, B7, C7, D7, E7, F7, G7, H7, A8, B8, C8, D8, E8, F8, G8, H8};

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

constexpr bool is_valid(Square square)
{
    return (square & 0x88) == 0;
}

constexpr Square value_of(File file, Rank rank)
{
    return Square((rank << 4) + file);
}

constexpr File get_file(Square square)
{
    return File(square & 0xF);
}

constexpr Rank get_rank(Square square)
{
    return Rank(square >> 4);
}

}  // namespace Squares

}  // namespace goldfish
