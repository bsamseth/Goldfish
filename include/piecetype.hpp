#pragma once

#include <array>

namespace goldfish {

class PieceType {
public:
    static const int MASK = 0x7;

    static const int PAWN = 0;
    static const int KNIGHT = 1;
    static const int BISHOP = 2;
    static const int ROOK = 3;
    static const int QUEEN = 4;
    static const int KING = 5;

    static const int NO_PIECE_TYPE = 6;

    static const int VALUES_SIZE = 6;
    static const std::array<int, VALUES_SIZE> values;

    // Piece values as defined by Larry Kaufman
    static const int PAWN_VALUE = 100;
    static const int KNIGHT_VALUE = 325;
    static const int BISHOP_VALUE = 325;
    static const int ROOK_VALUE = 500;
    static const int QUEEN_VALUE = 975;
    static const int KING_VALUE = 20000;

    static bool is_valid_promotion(int piecetype);

    static bool is_sliding(int piecetype);

    static int get_value(int piecetype);

    PieceType() = delete;

    ~PieceType() = delete;
};

}
