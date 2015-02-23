
#ifndef MOVE_H
#define MOVE_H
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



class Move {
    public:
    Move(Square from, Square to);
    Move(Square from, Square to, MoveFlag moveflag);
    unsigned int getInteger();
    Square getTo() const;
    Square getFrom() const;
    MoveFlag getFlag() const;
    void setTo(Square to);
    void setFrom(Square from);
    bool capture();
    bool quiet();
    bool castle();
    bool kingCastle();
    bool queenCastle();
    bool ep_capture();
    bool promotion();

    protected:
    unsigned int m_Move;
};

inline bool Move::capture() {
    return (getFlag() & 0x04);
}

inline bool Move::quiet() {
    return getFlag() == QUIET_MOVE;
}

inline bool Move::kingCastle() {
    return getFlag() == KING_CASTLE_MOVE;
}

inline bool Move::queenCastle() {
    return getFlag() == QUEEN_CASTLE_MOVE;
}

inline bool Move::castle() {
    return kingCastle() || queenCastle();
}

inline bool Move::ep_capture() {
    return getFlag() == ENPASSANT_CAPTURE_MOVE;
}

inline bool Move::promotion() {
    return (getFlag() & 0x08);
}



#endif
