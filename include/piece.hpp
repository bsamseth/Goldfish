#pragma once

#include <array>
#include <stdexcept>

#include "color.hpp"
#include "piecetype.hpp"

namespace goldfish {

enum Piece {
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
    NO_PIECE
};


namespace Pieces {

constexpr int MASK = 0x1F;

constexpr int VALUES_SIZE = 12;
constexpr std::array<Piece, VALUES_SIZE> values = {
    Piece::WHITE_PAWN, Piece::WHITE_KNIGHT, Piece::WHITE_BISHOP,
    Piece::WHITE_ROOK, Piece::WHITE_QUEEN, Piece::WHITE_KING,
    Piece::BLACK_PAWN, Piece::BLACK_KNIGHT, Piece::BLACK_BISHOP,
    Piece::BLACK_ROOK, Piece::BLACK_QUEEN, Piece::BLACK_KING
};

inline constexpr bool is_valid(Piece piece) {
    switch (piece) {
        case Piece::WHITE_PAWN:
        case Piece::WHITE_KNIGHT:
        case Piece::WHITE_BISHOP:
        case Piece::WHITE_ROOK:
        case Piece::WHITE_QUEEN:
        case Piece::WHITE_KING:
        case Piece::BLACK_PAWN:
        case Piece::BLACK_KNIGHT:
        case Piece::BLACK_BISHOP:
        case Piece::BLACK_ROOK:
        case Piece::BLACK_QUEEN:
        case Piece::BLACK_KING:
            return true;
        default:
            return false;
    }
}

inline Piece value_of(Color color, PieceType piecetype) {
    switch (color) {
        case Color::WHITE:
            switch (piecetype) {
                case PieceType::PAWN:
                    return Piece::WHITE_PAWN;
                case PieceType::KNIGHT:
                    return Piece::WHITE_KNIGHT;
                case PieceType::BISHOP:
                    return Piece::WHITE_BISHOP;
                case PieceType::ROOK:
                    return Piece::WHITE_ROOK;
                case PieceType::QUEEN:
                    return Piece::WHITE_QUEEN;
                case PieceType::KING:
                    return Piece::WHITE_KING;
                default:
                    throw std::exception();
            }
        case Color::BLACK:
            switch (piecetype) {
                case PieceType::PAWN:
                    return Piece::BLACK_PAWN;
                case PieceType::KNIGHT:
                    return Piece::BLACK_KNIGHT;
                case PieceType::BISHOP:
                    return Piece::BLACK_BISHOP;
                case PieceType::ROOK:
                    return Piece::BLACK_ROOK;
                case PieceType::QUEEN:
                    return Piece::BLACK_QUEEN;
                case PieceType::KING:
                    return Piece::BLACK_KING;
                default:
                    throw std::exception();
            }
        default:
            throw std::exception();
    }
}

inline PieceType get_type(Piece piece) {
    switch (piece) {
        case Piece::WHITE_PAWN:
        case Piece::BLACK_PAWN:
            return PieceType::PAWN;
        case Piece::WHITE_KNIGHT:
        case Piece::BLACK_KNIGHT:
            return PieceType::KNIGHT;
        case Piece::WHITE_BISHOP:
        case Piece::BLACK_BISHOP:
            return PieceType::BISHOP;
        case Piece::WHITE_ROOK:
        case Piece::BLACK_ROOK:
            return PieceType::ROOK;
        case Piece::WHITE_QUEEN:
        case Piece::BLACK_QUEEN:
            return PieceType::QUEEN;
        case Piece::WHITE_KING:
        case Piece::BLACK_KING:
            return PieceType::KING;
        default:
            throw std::exception();
    }
}

inline Color get_color(Piece piece) {
    switch (piece) {
        case Piece::WHITE_PAWN:
        case Piece::WHITE_KNIGHT:
        case Piece::WHITE_BISHOP:
        case Piece::WHITE_ROOK:
        case Piece::WHITE_QUEEN:
        case Piece::WHITE_KING:
            return Color::WHITE;
        case Piece::BLACK_PAWN:
        case Piece::BLACK_KNIGHT:
        case Piece::BLACK_BISHOP:
        case Piece::BLACK_ROOK:
        case Piece::BLACK_QUEEN:
        case Piece::BLACK_KING:
            return Color::BLACK;
        default:
            throw std::exception();
    }
}

}

}
