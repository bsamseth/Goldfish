#ifndef POSITION_H
#define POSITION_H

#include <string>
#include <vector>

#include "types.h"
#include "move.h"

using std::string;
using std::vector;


struct StateInfo {
    Piece lastMove_originPiece;
    Piece lastMove_destinationPiece;
    StateInfo* previous;
};


class Position {
    public:
    // constructors
    Position();
    void setFromFEN(std::string fen);
    void doMove(Move m);
    void undoMove();
    void doNullMove();
    // getters
    Bitboard getBoardForColor(Color c);
    // helper functions
    void putPiece(Square sq, Piece p);
    void putPiece(Square sq, PieceType pt, Color c);
    void clear();
    
    // pieces
    bool occupied(Square s);
    // public fields
    Bitboard pieces[NUMBER_OF_COLORS][NUMBER_OF_PIECE_TYPES];
    Piece board[NUMBER_OF_SQUARES];
    StateInfo* stateInfo;

    protected:
    vector<Move> moveList;
    Color sideToMove;
    CastlingRights castlingRights;
    Square enpassantTarget;
    unsigned halfmoveClock;
    unsigned fullmoveNumber;

};

#endif
