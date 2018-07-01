#pragma once

#include <array>

namespace goldfish {

class Piece {
public:
    static const int MASK = 0x1F;

    static const int WHITE_PAWN = 0;
    static const int WHITE_KNIGHT = 1;
    static const int WHITE_BISHOP = 2;
    static const int WHITE_ROOK = 3;
    static const int WHITE_QUEEN = 4;
    static const int WHITE_KING = 5;
    static const int BLACK_PAWN = 6;
    static const int BLACK_KNIGHT = 7;
    static const int BLACK_BISHOP = 8;
    static const int BLACK_ROOK = 9;
    static const int BLACK_QUEEN = 10;
    static const int BLACK_KING = 11;

    static const int NO_PIECE = 12;

    static const int VALUES_SIZE = 12;
    static const std::array<int, VALUES_SIZE> values;

    static bool is_valid(int piece);

    static int value_of(int color, int piecetype);

    static int get_type(int piece);

    static int get_color(int piece);

    Piece() = delete;

    ~Piece() = delete;
};

}
