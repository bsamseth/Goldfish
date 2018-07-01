#pragma once

#include <array>
#include <vector>

namespace goldfish {

class Square {
public:
    static const int MASK = 0x7F;

    static const int A1 = 0, A2 = 16;
    static const int B1 = 1, B2 = 17;
    static const int C1 = 2, C2 = 18;
    static const int D1 = 3, D2 = 19;
    static const int E1 = 4, E2 = 20;
    static const int F1 = 5, F2 = 21;
    static const int G1 = 6, G2 = 22;
    static const int H1 = 7, H2 = 23;

    static const int A3 = 32, A4 = 48;
    static const int B3 = 33, B4 = 49;
    static const int C3 = 34, C4 = 50;
    static const int D3 = 35, D4 = 51;
    static const int E3 = 36, E4 = 52;
    static const int F3 = 37, F4 = 53;
    static const int G3 = 38, G4 = 54;
    static const int H3 = 39, H4 = 55;

    static const int A5 = 64, A6 = 80;
    static const int B5 = 65, B6 = 81;
    static const int C5 = 66, C6 = 82;
    static const int D5 = 67, D6 = 83;
    static const int E5 = 68, E6 = 84;
    static const int F5 = 69, F6 = 85;
    static const int G5 = 70, G6 = 86;
    static const int H5 = 71, H6 = 87;

    static const int A7 = 96, A8 = 112;
    static const int B7 = 97, B8 = 113;
    static const int C7 = 98, C8 = 114;
    static const int D7 = 99, D8 = 115;
    static const int E7 = 100, E8 = 116;
    static const int F7 = 101, F8 = 117;
    static const int G7 = 102, G8 = 118;
    static const int H7 = 103, H8 = 119;

    static const int NO_SQUARE = 127;

    static const int VALUES_LENGTH = 128;
    static const int VALUES_SIZE = 64;
    static const std::array<int, VALUES_SIZE> values;

    // These are our move directions
    // N = north, E = east, S = south, W = west
    static const int N = 16;
    static const int E = 1;
    static const int S = -16;
    static const int W = -1;
    static const int NE = N + E;
    static const int SE = S + E;
    static const int SW = S + W;
    static const int NW = N + W;

    static const std::vector<std::vector<int>> pawn_directions;
    static const std::vector<int> knight_directions;
    static const std::vector<int> bishop_directions;
    static const std::vector<int> rook_directions;
    static const std::vector<int> queen_directions;
    static const std::vector<int> king_directions;

    static bool is_valid(int square);

    static int value_of(int file, int rank);

    static int get_file(int square);

    static int get_rank(int square);

    Square() = delete;

    ~Square() = delete;
};

}
