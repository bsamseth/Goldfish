#include "move.hpp"

namespace goldfish {

int Move::value_of(int type, int origin_square, int target_square, int origin_piece, int target_piece, int promotion) {
    int move = 0;

    // Encode type
    move |= type << TYPE_SHIFT;

    // Encode origin square
    move |= origin_square << ORIGIN_SQUARE_SHIFT;

    // Encode target square
    move |= target_square << TARGET_SQUARE_SHIFT;

    // Encode origin piece
    move |= origin_piece << ORIGIN_PIECE_SHIFT;

    // Encode target piece
    move |= target_piece << TARGET_PIECE_SHIFT;

    // Encode promotion
    move |= promotion << PROMOTION_SHIFT;

    return move;
}

int Move::get_type(int move) {
    return (move & TYPE_MASK) >> TYPE_SHIFT;
}

int Move::get_origin_square(int move) {
    return (move & ORIGIN_SQUARE_MASK) >> ORIGIN_SQUARE_SHIFT;
}

int Move::get_target_square(int move) {
    return (move & TARGET_SQUARE_MASK) >> TARGET_SQUARE_SHIFT;
}

int Move::get_origin_piece(int move) {
    return (move & ORIGIN_PIECE_MASK) >> ORIGIN_PIECE_SHIFT;
}

int Move::get_target_piece(int move) {
    return (move & TARGET_PIECE_MASK) >> TARGET_PIECE_SHIFT;
}

int Move::get_promotion(int move) {
    return (move & PROMOTION_MASK) >> PROMOTION_SHIFT;
}

}
