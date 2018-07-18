#pragma once

#include "square.hpp"
#include "piece.hpp"
#include "piecetype.hpp"
#include "movetype.hpp"


/**
 * A move is encoded in an integer. The fields are represented by
 * the following bits.
 * <p/>
 * <code> 0 -  2</code>: type (required)
 * <code> 3 -  9</code>: origin square (required)
 * <code>10 - 16</code>: target square (required)
 * <code>17 - 21</code>: origin piece (required)
 * <code>22 - 26</code>: target piece (optional)
 * <code>27 - 29</code>: promotion type (optional)
 */
using Move = int;

namespace goldfish::Moves {

// These are our bit masks
constexpr int TYPE_SHIFT = 0;
constexpr int TYPE_MASK = MoveTypes::MASK << TYPE_SHIFT;
constexpr int ORIGIN_SQUARE_SHIFT = 3;
constexpr int ORIGIN_SQUARE_MASK = Squares::MASK << ORIGIN_SQUARE_SHIFT;
constexpr int TARGET_SQUARE_SHIFT = 10;
constexpr int TARGET_SQUARE_MASK = Squares::MASK << TARGET_SQUARE_SHIFT;
constexpr int ORIGIN_PIECE_SHIFT = 17;
constexpr int ORIGIN_PIECE_MASK = Pieces::MASK << ORIGIN_PIECE_SHIFT;
constexpr int TARGET_PIECE_SHIFT = 22;
constexpr int TARGET_PIECE_MASK = Pieces::MASK << TARGET_PIECE_SHIFT;
constexpr int PROMOTION_SHIFT = 27;
constexpr int PROMOTION_MASK = PieceTypes::MASK << PROMOTION_SHIFT;

// We don't use 0 as a null value to protect against errors.
constexpr Move NO_MOVE =
      (MoveType::NO_MOVE_TYPE << TYPE_SHIFT)
    | (Square::NO_SQUARE << ORIGIN_SQUARE_SHIFT)
    | (Square::NO_SQUARE << TARGET_SQUARE_SHIFT)
    | (Piece::NO_PIECE << ORIGIN_PIECE_SHIFT)
    | (Piece::NO_PIECE << TARGET_PIECE_SHIFT)
    | (PieceType::NO_PIECE_TYPE << PROMOTION_SHIFT);

constexpr Move value_of(int type, Square origin_square, Square target_square,
                       Piece origin_piece, Piece target_piece, PieceType promotion) {
    Move move = 0;
    move |= type << TYPE_SHIFT;
    move |= origin_square << ORIGIN_SQUARE_SHIFT;
    move |= target_square << TARGET_SQUARE_SHIFT;
    move |= origin_piece << ORIGIN_PIECE_SHIFT;
    move |= target_piece << TARGET_PIECE_SHIFT;
    move |= promotion << PROMOTION_SHIFT;
    return move;
}

inline constexpr MoveType get_type(Move move) {
    return static_cast<MoveType>((move & TYPE_MASK) >> TYPE_SHIFT);
}

inline constexpr Square get_origin_square(Move move) {
    return static_cast<Square>((move & ORIGIN_SQUARE_MASK) >> ORIGIN_SQUARE_SHIFT);
}

inline constexpr Square get_target_square(Move move) {
    return static_cast<Square>((move & TARGET_SQUARE_MASK) >> TARGET_SQUARE_SHIFT);
}

inline constexpr Piece get_origin_piece(int move) {
    return static_cast<Piece>((move & ORIGIN_PIECE_MASK) >> ORIGIN_PIECE_SHIFT);
}

inline constexpr Piece get_target_piece(int move) {
    return static_cast<Piece>((move & TARGET_PIECE_MASK) >> TARGET_PIECE_SHIFT);
}

inline constexpr PieceType get_promotion(int move) {
    return static_cast<PieceType>((move & PROMOTION_MASK) >> PROMOTION_SHIFT);
}

}
