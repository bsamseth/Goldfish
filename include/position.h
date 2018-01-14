#ifndef POSITION_H
#define POSITION_H

#include <string>
#include <vector>

#include "types.h"
#include "move.h"
#include "stateinfo.h"

using std::string;
using std::vector;

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

StateInfo* const rootState = new StateInfo(true);

class Position {
    public:
        Position();
        Position(std::string);
        void initHashing();
        void setFromFEN(std::string fen);
        void doMove(Move m);
        void undoMove();
        void doNullMove();
        int score() const;

        // getters
        Bitboard getBoardForColor(Color c) const;
        Color getSideToMove() const;

        // helper functions
        void putPiece(Square sq, Piece p);
        void putPiece(Square sq, PieceType pt, Color c);
        void silentDoMove(Move m);
        void moveCastleRook(Square from, Square to);
        void clear();
        string str() const;

        // pieces
        bool occupied(Square s) const;
        bool occupied(Square s, Color c) const;

        // public fields
        Bitboard pieces[NUMBER_OF_COLORS-1][NUMBER_OF_PIECE_TYPES];
        Piece board[NUMBER_OF_SQUARES];
        StateInfo* stateInfo;

        Square enpassantTarget;
        Square kingpassantTarget;
        CastlingRights castlingRights;
        unsigned halfmoveClock;
        unsigned fullmoveNumber;
        Color sideToMove;
        vector<Move> moveList;
        Key hash;
};


inline Color Position::getSideToMove() const {
    return sideToMove;
}

#endif
