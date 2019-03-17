#pragma once

#include <array>
#include "value.hpp"
#include "operations.hpp"

namespace goldfish {

enum PieceType {
  NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
  ALL_PIECES = 0,
  PIECE_TYPE_NB = 8
};

ENABLE_INCR_OPERATORS_ON(PieceType)

namespace PieceTypes {

constexpr int VALUES_SIZE = 6;
constexpr std::array<PieceType, VALUES_SIZE> values = {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

constexpr bool is_valid_promotion(PieceType pt) {
    return KNIGHT <= pt && pt <= QUEEN;
}

constexpr bool is_sliding(PieceType pt) {
    return BISHOP <= pt && pt <= QUEEN;
}

constexpr Value value_of(PieceType pt) {
    switch (pt) {
        case PAWN:
            return Value::PAWN_VALUE;
        case KNIGHT:
            return Value::KNIGHT_VALUE;
        case BISHOP:
            return Value::BISHOP_VALUE;
        case ROOK:
            return Value::ROOK_VALUE;
        case QUEEN:
            return Value::QUEEN_VALUE;
        case KING:
            return Value::KING_VALUE;
        default:
            throw std::invalid_argument("Bad piecetype");
    }
}

}
}
