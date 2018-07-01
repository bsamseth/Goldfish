#pragma once

namespace goldfish {

class MoveType {
public:
    static const int MASK = 0x7;

    static const int NORMAL = 0;
    static const int PAWN_DOUBLE = 1;
    static const int PAWN_PROMOTION = 2;
    static const int EN_PASSANT = 3;
    static const int CASTLING = 4;

    static const int NO_MOVE_TYPE = 5;

    MoveType() = delete;

    ~MoveType() = delete;
};

}
