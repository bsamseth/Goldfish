#pragma once

#include "value.hpp"

#include <array>

namespace goldfish
{
enum PieceType
{
    PAWN = 0,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NO_PIECE_TYPE
};

namespace PieceTypes
{
constexpr int MASK = 0x7;

constexpr int                                VALUES_SIZE = 6;
constexpr std::array<PieceType, VALUES_SIZE> values
    = {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

constexpr bool is_valid_promotion(PieceType piecetype)
{
    switch (piecetype)
    {
        case KNIGHT:
        case BISHOP:
        case ROOK:
        case QUEEN: return true;
        default: return false;
    }
}

constexpr bool is_sliding(PieceType piecetype)
{
    switch (piecetype)
    {
        case BISHOP:
        case ROOK:
        case QUEEN: return true;
        case PAWN:
        case KNIGHT:
        case KING: return false;
        default: throw std::exception();
    }
}

constexpr Value get_value(PieceType piecetype)
{
    switch (piecetype)
    {
        case PAWN: return Value::PAWN_VALUE;
        case KNIGHT: return Value::KNIGHT_VALUE;
        case BISHOP: return Value::BISHOP_VALUE;
        case ROOK: return Value::ROOK_VALUE;
        case QUEEN: return Value::QUEEN_VALUE;
        case KING: return Value::KING_VALUE;
        default: throw std::invalid_argument("Bad piecetype");
    }
}

}  // namespace PieceTypes
}  // namespace goldfish
