#pragma once

#include <array>

namespace goldfish {

class Castling {
public:
    static const int WHITE_KING_SIDE = 1 << 0;
    static const int WHITE_QUEEN_SIDE = 1 << 1;
    static const int BLACK_KING_SIDE = 1 << 2;
    static const int BLACK_QUEEN_SIDE = 1 << 3;

    static const int NO_CASTLING = 0;

    static const int VALUES_LENGTH = 16;

    static int value_of(int color, int castlingtype);

    static int get_type(int castling);

    static int get_color(int castling);

    Castling() = delete;

    ~Castling() = delete;
};

}
