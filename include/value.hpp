#pragma once

#include <cmath>
#include <array>
#include <stdexcept>

#include "file.hpp"
#include "depth.hpp"
#include "score.hpp"
#include "piecetype.hpp"
#include "square.hpp"
#include "color.hpp"
#include "piece.hpp"
#include "operations.hpp"

namespace goldfish {

enum Value : int {
    // Piece values taken from taken from Stockfish @ 759b3c7.
    VALUE_PAWN   = 171,
    VALUE_KNIGHT = 764,
    VALUE_BISHOP = 826,
    VALUE_ROOK   = 1282,
    VALUE_QUEEN  = 2500,
    // King is worth more than the maximum of 9 queens and 2 knights+bishops+rooks. Set to
    // twice this, so that having one king means more material than opponent could have without
    // their king.
    VALUE_KING   = 2 * (8 * VALUE_QUEEN + 2 * (VALUE_KNIGHT + VALUE_BISHOP + VALUE_ROOK)) + 1,

    VALUE_ZERO = 0,
    VALUE_DRAW = 0,
    // Value of mate is so that if we have a king, and opponent has maximum material, but no king,
    // then the material difference gives a mate to us.
    VALUE_MATE = VALUE_KING / 2,
    VALUE_INFINITE = VALUE_MATE + 1,
    VALUE_NONE = VALUE_INFINITE + 1,
    VALUE_MATE_THRESHOLD = VALUE_MATE - Depth::MAX_PLY,
};

ENABLE_FULL_OPERATORS_ON(Value);

namespace Values {

inline constexpr Value get_value(PieceType pt) {
    switch (pt) {
        case PieceType::PAWN: return Value::VALUE_PAWN;
        case PieceType::KNIGHT: return Value::VALUE_KNIGHT;
        case PieceType::BISHOP: return Value::VALUE_BISHOP;
        case PieceType::ROOK: return Value::VALUE_ROOK;
        case PieceType::QUEEN: return Value::VALUE_QUEEN;
        case PieceType::KING: return Value::VALUE_KING;
        default: throw std::exception();
    }
}

// Piece square bonus values, taken from Stockfish @ 759b3c7.
// Values are given for files A-D, and is symmetric for E-H,
// and symmetric for black.
constexpr Score piece_square_bonus[PieceTypes::VALUES_SIZE][Ranks::VALUES_SIZE][Files::VALUES_SIZE/2] = {
    { // Pawn
        { Score(  0), Score(  0), Score(  0), Score( 0) },
        { Score(-11), Score(  6), Score(  7), Score( 3) },
        { Score(-18), Score( -2), Score( 19), Score(24) },
        { Score(-17), Score( -9), Score( 20), Score(35) },
        { Score( -6), Score(  5), Score(  3), Score(21) },
        { Score( -6), Score( -8), Score( -6), Score(-2) },
        { Score( -4), Score( 20), Score( -8), Score(-4) },
        { Score(  0), Score(  0), Score(  0), Score( 0) }
    },
    { // Knight
        { Score(-161), Score(-96), Score(-80), Score(-73) },
        { Score( -83), Score(-43), Score(-21), Score(-10) },
        { Score( -71), Score(-22), Score(  0), Score(  9) },
        { Score( -25), Score( 18), Score( 43), Score( 47) },
        { Score( -26), Score( 16), Score( 38), Score( 50) },
        { Score( -11), Score( 37), Score( 56), Score( 65) },
        { Score( -63), Score(-19), Score(  5), Score( 14) },
        { Score(-195), Score(-67), Score(-42), Score(-29) }
    },
    { // Bishop
        { Score(-44), Score(-13), Score(-25), Score(-34) },
        { Score(-20), Score( 20), Score( 12), Score(  1) },
        { Score( -9), Score( 27), Score( 21), Score( 11) },
        { Score(-11), Score( 28), Score( 21), Score( 10) },
        { Score(-11), Score( 27), Score( 16), Score(  9) },
        { Score(-17), Score( 16), Score( 12), Score(  2) },
        { Score(-23), Score( 17), Score(  6), Score( -2) },
        { Score(-35), Score(-11), Score(-19), Score(-29) }
    },
    { // Rook
        { Score(-25), Score(-16), Score(-16), Score(-9) },
        { Score(-21), Score( -8), Score( -3), Score( 0) },
        { Score(-21), Score( -9), Score( -4), Score( 2) },
        { Score(-22), Score( -6), Score( -1), Score( 2) },
        { Score(-22), Score( -7), Score(  0), Score( 1) },
        { Score(-21), Score( -7), Score(  0), Score( 2) },
        { Score(-12), Score(  4), Score(  8), Score(12) },
        { Score(-23), Score(-15), Score(-11), Score(-5) }
    },
    { // Queen
        { Score( 0), Score(-4), Score(-3), Score(-1) },
        { Score(-4), Score( 6), Score( 9), Score( 8) },
        { Score(-2), Score( 6), Score( 9), Score( 9) },
        { Score(-1), Score( 8), Score(10), Score( 7) },
        { Score(-3), Score( 9), Score( 8), Score( 7) },
        { Score(-2), Score( 6), Score( 8), Score(10) },
        { Score(-2), Score( 7), Score( 7), Score( 6) },
        { Score(-1), Score(-4), Score(-1), Score( 0) }
    },
    { // King
        { Score(267), Score(320), Score(270), Score(195) },
        { Score(264), Score(304), Score(238), Score(180) },
        { Score(200), Score(245), Score(176), Score(110) },
        { Score(177), Score(185), Score(148), Score(110) },
        { Score(149), Score(177), Score(115), Score( 66) },
        { Score(118), Score(159), Score( 84), Score( 41) },
        { Score( 87), Score(128), Score( 63), Score( 20) },
        { Score( 63), Score( 88), Score( 47), Score(  0) }
    }
};


extern std::array<std::array<Score, Squares::VALUES_LENGTH>, Pieces::VALUES_SIZE> psqt;

/**
 * Initialize the piece square tables, joining the bonus values with the piece values.
 */
void init_psqt();

inline constexpr bool is_checkmate(int value) {
    int absvalue = std::abs(value);
    return absvalue >= Value::VALUE_MATE_THRESHOLD && absvalue <= Value::VALUE_MATE;
}

}
}
