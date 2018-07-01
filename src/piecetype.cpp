#include "piecetype.hpp"

namespace goldfish {

const std::array<int, PieceType::VALUES_SIZE> PieceType::values = {
        PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

bool PieceType::is_valid_promotion(int piecetype) {
    switch (piecetype) {
        case KNIGHT:
        case BISHOP:
        case ROOK:
        case QUEEN:
            return true;
        default:
            return false;
    }
}

bool PieceType::is_sliding(int piecetype) {
    switch (piecetype) {
        case BISHOP:
        case ROOK:
        case QUEEN:
            return true;
        case PAWN:
        case KNIGHT:
        case KING:
            return false;
        default:
            throw std::exception();
    }
}

int PieceType::get_value(int piecetype) {
    switch (piecetype) {
        case PAWN:
            return PAWN_VALUE;
        case KNIGHT:
            return KNIGHT_VALUE;
        case BISHOP:
            return BISHOP_VALUE;
        case ROOK:
            return ROOK_VALUE;
        case QUEEN:
            return QUEEN_VALUE;
        case KING:
            return KING_VALUE;
        default:
            throw std::invalid_argument("Bad piecetype");
    }
}

}
