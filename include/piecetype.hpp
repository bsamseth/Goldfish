#pragma once

#include <array>

namespace goldfish {

enum PieceType {
    PAWN, KNIGHT, BISHOP,
    ROOK, QUEEN, KING,
    NO_PIECE_TYPE
};

namespace PieceTypes {

constexpr int MASK = 0x7;

constexpr int VALUES_SIZE = 6;
constexpr std::array<PieceType, VALUES_SIZE> values{
    PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP,
    PieceType::ROOK, PieceType::QUEEN, PieceType::KING
};

// Piece values as defined by Larry Kaufman
constexpr int PAWN_VALUE = 100;
constexpr int KNIGHT_VALUE = 325;
constexpr int BISHOP_VALUE = 325;
constexpr int ROOK_VALUE = 500;
constexpr int QUEEN_VALUE = 975;
constexpr int KING_VALUE = 20000;

inline constexpr bool is_valid_promotion(PieceType piecetype) {
    switch (piecetype) {
        case PieceType::KNIGHT:
        case PieceType::BISHOP:
        case PieceType::ROOK:
        case PieceType::QUEEN:
            return true;
        default:
            return false;
    }
}

inline constexpr bool is_sliding(PieceType piecetype) {
    switch (piecetype) {
        case PieceType::BISHOP:
        case PieceType::ROOK:
        case PieceType::QUEEN:
            return true;
        case PieceType::PAWN:
        case PieceType::KNIGHT:
        case PieceType::KING:
            return false;
        default:
            throw std::exception();
    }
}

inline constexpr int get_value(PieceType piecetype) {
    switch (piecetype) {
        case PieceType::PAWN:
            return PAWN_VALUE;
        case PieceType::KNIGHT:
            return KNIGHT_VALUE;
        case PieceType::BISHOP:
            return BISHOP_VALUE;
        case PieceType::ROOK:
            return ROOK_VALUE;
        case PieceType::QUEEN:
            return QUEEN_VALUE;
        case PieceType::KING:
            return KING_VALUE;
        default:
            throw std::exception();
    }
}

}

}
