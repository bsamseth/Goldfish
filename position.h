#ifndef POSITION_H
#define POSITION_H

#include <string>
#include <vector>

#include "types.h"
#include "move.h"

using std::string;
using std::vector;

class Position {
    
    vector<Move> moveList;
    Color sideToMove;

    public:
    // constructors
    Position();


    void setFromFEN(std::string fen);
    void doMove(Move m);
    void undoMove(Move m);
    void doNullMove();

    // getters
    Bitboard getBoardForColor(Color c);
    
    // helper functions
    void putPiece(Square sq, PieceType pt, Color c);
    void clear();
    // fields
    Bitboard pieces[NUMBER_OF_COLORS][NUMBER_OF_PIECE_TYPES];
    Piece board[NUMBER_OF_SQUARES];
    CastlingRights castlingRights;
    Square enpassantTarget;
    unsigned halfmoveClock;
    unsigned fullmoveNumber;
};

#endif
