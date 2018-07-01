#include "square.hpp"

namespace goldfish {

const std::array<int, Square::VALUES_SIZE> Square::values = {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8
};

const std::vector<std::vector<int>> Square::pawn_directions = {
        {N, NE, NW}, // Color::WHITE
        {S, SE, SW}  // Color::BLACK
};
const std::vector<int> Square::knight_directions = {
        N + N + E,
        N + N + W,
        N + E + E,
        N + W + W,
        S + S + E,
        S + S + W,
        S + E + E,
        S + W + W
};
const std::vector<int> Square::bishop_directions = {
        NE, NW, SE, SW
};
const std::vector<int> Square::rook_directions = {
        N, E, S, W
};
const std::vector<int> Square::queen_directions = {
        N, E, S, W,
        NE, NW, SE, SW
};
const std::vector<int> Square::king_directions = {
        N, E, S, W,
        NE, NW, SE, SW
};

bool Square::is_valid(int square) {
    return (square & 0x88) == 0;
}

int Square::value_of(int file, int rank) {
    return (rank << 4) + file;
}

int Square::get_file(int square) {
    return square & 0xF;
}

int Square::get_rank(int square) {
    return square >> 4;
}

}

