#pragma once

#include <array>
#include <vector>

#include "file.hpp"
#include "rank.hpp"

namespace goldfish {

enum class Square {
    A1 = 0  , B1, C1, D1, E1, F1, G1, H1,
    A2 = 16 , B2, C2, D2, E2, F2, G2, H2,
    A3 = 32 , B3, C3, D3, E3, F3, G3, H3,
    A4 = 48 , B4, C4, D4, E4, F4, G4, H4,
    A5 = 64 , B5, C5, D5, E5, F5, G5, H5,
    A6 = 80 , B6, C6, D6, E6, F6, G6, H6,
    A7 = 96 , B7, C7, D7, E7, F7, G7, H7,
    A8 = 112, B8, C8, D8, E8, F8, G8, H8,
    NO_SQUARE = 127
};

namespace Squares {

constexpr int MASK = 0x7F;

constexpr int VALUES_LENGTH = 128;
constexpr int VALUES_SIZE = 64;
constexpr std::array<Square, VALUES_SIZE> values{
    Square::A1, Square::B1, Square::C1, Square::D1, Square::E1, Square::F1, Square::G1, Square::H1,
    Square::A2, Square::B2, Square::C2, Square::D2, Square::E2, Square::F2, Square::G2, Square::H2,
    Square::A3, Square::B3, Square::C3, Square::D3, Square::E3, Square::F3, Square::G3, Square::H3,
    Square::A4, Square::B4, Square::C4, Square::D4, Square::E4, Square::F4, Square::G4, Square::H4,
    Square::A5, Square::B5, Square::C5, Square::D5, Square::E5, Square::F5, Square::G5, Square::H5,
    Square::A6, Square::B6, Square::C6, Square::D6, Square::E6, Square::F6, Square::G6, Square::H6,
    Square::A7, Square::B7, Square::C7, Square::D7, Square::E7, Square::F7, Square::G7, Square::H7,
    Square::A8, Square::B8, Square::C8, Square::D8, Square::E8, Square::F8, Square::G8, Square::H8
};

// These are our move directions
// N = north, E = east, S = south, W = west
constexpr int N = 16;
constexpr int E = 1;
constexpr int S = -16;
constexpr int W = -1;
constexpr int NE = N + E;
constexpr int SE = S + E;
constexpr int SW = S + W;
constexpr int NW = N + W;

const std::vector<std::vector<int>> pawn_directions{
    {N, NE, NW}, // Color::WHITE
    {S, SE, SW}  // Color::BLACK
};

const std::vector<int> knight_directions = {
    N + N + E,
    N + N + W,
    N + E + E,
    N + W + W,
    S + S + E,
    S + S + W,
    S + E + E,
    S + W + W
};

const std::vector<int> bishop_directions = {
    NE, NW, SE, SW
};

const std::vector<int> rook_directions = {
    N, E, S, W
};

const std::vector<int> queen_directions = {
    N, E, S, W,
    NE, NW, SE, SW
};

const std::vector<int> king_directions = {
    N, E, S, W,
    NE, NW, SE, SW
};

inline constexpr bool is_valid(Square square) {
    return (static_cast<int>(square) & 0x88) == 0;
}

inline constexpr Square value_of(File file, Rank rank) {
    return static_cast<Square>((static_cast<int>(rank) << 4) + static_cast<int>(file));
}

inline constexpr File get_file(Square square) {
    return static_cast<File>(static_cast<int>(square) & 0xF);
}

inline constexpr Rank get_rank(Square square) {
    return static_cast<Rank>(static_cast<int>(square) >> 4);
}

}

}
