#pragma once

namespace goldfish::MoveType {

enum MoveType {
    NORMAL, PAWN_DOUBLE, PAWN_PROMOTION,
    EN_PASSANT, CASTLING, NO_MOVE_TYPE,
};

constexpr int MASK = 0x7;

}
