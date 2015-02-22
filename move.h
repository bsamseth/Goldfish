
#ifndef MOVE_H
#define MOVE_H
#include "types.h"


/* Move encoding taken from Stockfish

A move needs 16 bits to be stored

bit  0- 5: destination square (from 0 to 63)
bit  6-11: origin square (from 0 to 63)
bit 12-13: promotion piece type - 2 (from KNIGHT-2 to QUEEN-2)
bit 14-15: special move flag: promotion (1), en passant (2), castling (3)
NOTE: EN-PASSANT bit is set only when a pawn can be captured

Special cases are MOVE_NONE and MOVE_NULL. We can sneak these in because in
any normal move destination square is always different from origin square
while MOVE_NONE and MOVE_NULL have the same origin and destination square.*/




class Move {
    public:
    Move(Square from, Square to);
    Move(Square from, Square to, PieceType promotionType, unsigned specialFlag);
    Square getTo() const;
    Square getFrom() const;
    unsigned int getFlags() const;
    void setTo(Square to);
    void setFrom(Square from);
    bool capture();
    protected:
    unsigned int m_Move;
};

#endif
