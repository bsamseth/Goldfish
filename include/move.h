
#ifndef MOVE_H
#define MOVE_H

#include <string>
#include "types.h"

/*
   A move is encoded as a 16-bit word, see https://chessprogramming.wikispaces.com/Encoding+Moves.

   bit   0-5: origin square (from 0 to 63)
   bit  6-11: destination square (from 0 to 63)
   bit 12-15: special flag

   The possible flags are listed in the enum MoveFlags

*/

enum MoveFlag {
    QUIET_MOVE = 0, DOUBLE_PAWN_PUSH_MOVE, KING_CASTLE_MOVE,
    QUEEN_CASTLE_MOVE, CAPTURE_MOVE, ENPASSANT_CAPTURE_MOVE,
    KNIGHT_PROMO_MOVE = 8, BISHOP_PROMO_MOVE, ROOK_PROMO_MOVE,
    QUEEN_PROMO_MOVE, KNIGHT_PROMO_CAPTURE_MOVE,
    BISHOP_PROMO_CAPTURE_MOVE, ROOK_PROMO_CAPTURE_MOVE,
    QUEEN_PROMO_CAPTURE_MOVE
};

const PieceType move_to_promotion_piece [QUEEN_PROMO_CAPTURE_MOVE + 1] = {
    NO_PIECE_TYPE, NO_PIECE_TYPE, NO_PIECE_TYPE, NO_PIECE_TYPE,
    NO_PIECE_TYPE, NO_PIECE_TYPE, NO_PIECE_TYPE, NO_PIECE_TYPE,
    KNIGHT, BISHOP, ROOK, QUEEN, KNIGHT, BISHOP, ROOK, QUEEN
};

const unsigned NO_MOVE = 0;

class Move {
    public:
        Move();
        Move(Square from, Square to);
        Move(Square from, Square to, MoveFlag moveflag);
        unsigned int getInteger() const;
        Square getTo() const;
        Square getFrom() const;
        MoveFlag getFlag() const;
        void setTo(Square to);
        void setFrom(Square from);
        bool capture() const;
        bool quiet() const;
        bool castle() const;
        bool kingCastle() const;
        bool queenCastle() const;
        bool ep_capture() const;
        bool promotion() const;
        bool doublePawnPush() const;
        std::string str() const;
    protected:
        unsigned int m_Move;
};

inline std::string Move::str() const {
    return SquareName[getFrom()] + SquareName[getTo()];
}

inline bool Move::doublePawnPush() const{
    return getFlag() == DOUBLE_PAWN_PUSH_MOVE;
}

inline bool Move::capture() const {
    return (getFlag() & 0x04);
}

inline bool Move::quiet() const {
    return getFlag() == QUIET_MOVE;
}

inline bool Move::kingCastle() const {
    return getFlag() == KING_CASTLE_MOVE;
}

inline bool Move::queenCastle() const {
    return getFlag() == QUEEN_CASTLE_MOVE;
}

inline bool Move::castle() const {
    return kingCastle() || queenCastle();
}

inline bool Move::ep_capture() const {
    return getFlag() == ENPASSANT_CAPTURE_MOVE;
}

inline bool Move::promotion() const {
    return (getFlag() & 0x08);
}

#endif

