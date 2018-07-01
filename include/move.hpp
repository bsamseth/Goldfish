#pragma once

#include "square.hpp"
#include "piece.hpp"
#include "piecetype.hpp"
#include "movetype.hpp"

namespace goldfish {

/**
 * This class represents a move as a int value. The fields are represented by
 * the following bits.
 * <p/>
 * <code> 0 -  2</code>: type (required)
 * <code> 3 -  9</code>: origin square (required)
 * <code>10 - 16</code>: target square (required)
 * <code>17 - 21</code>: origin piece (required)
 * <code>22 - 26</code>: target piece (optional)
 * <code>27 - 29</code>: promotion type (optional)
 */
class Move {
private:
    // These are our bit masks
    static const int TYPE_SHIFT = 0;
    static const int TYPE_MASK = MoveType::MASK << TYPE_SHIFT;
    static const int ORIGIN_SQUARE_SHIFT = 3;
    static const int ORIGIN_SQUARE_MASK = Square::MASK << ORIGIN_SQUARE_SHIFT;
    static const int TARGET_SQUARE_SHIFT = 10;
    static const int TARGET_SQUARE_MASK = Square::MASK << TARGET_SQUARE_SHIFT;
    static const int ORIGIN_PIECE_SHIFT = 17;
    static const int ORIGIN_PIECE_MASK = Piece::MASK << ORIGIN_PIECE_SHIFT;
    static const int TARGET_PIECE_SHIFT = 22;
    static const int TARGET_PIECE_MASK = Piece::MASK << TARGET_PIECE_SHIFT;
    static const int PROMOTION_SHIFT = 27;
    static const int PROMOTION_MASK = PieceType::MASK << PROMOTION_SHIFT;

public:
    // We don't use 0 as a null value to protect against errors.
    static const int NO_MOVE = (MoveType::NO_MOVE_TYPE << TYPE_SHIFT)
                               | (Square::NO_SQUARE << ORIGIN_SQUARE_SHIFT)
                               | (Square::NO_SQUARE << TARGET_SQUARE_SHIFT)
                               | (Piece::NO_PIECE << ORIGIN_PIECE_SHIFT)
                               | (Piece::NO_PIECE << TARGET_PIECE_SHIFT)
                               | (PieceType::NO_PIECE_TYPE << PROMOTION_SHIFT);

    static int
    value_of(int type, int origin_square, int target_square, int origin_piece, int target_piece, int promotion);

    static int get_type(int move);

    static int get_origin_square(int move);

    static int get_target_square(int move);

    static int get_origin_piece(int move);

    static int get_target_piece(int move);

    static int get_promotion(int move);

    Move() = delete;

    ~Move() = delete;
};

}
