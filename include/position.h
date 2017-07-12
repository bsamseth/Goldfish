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
        // constructors
        Position();
        Position(std::string);
        void setFromFEN(std::string fen);
        void doMove(Move m);
        void undoMove();
        void doNullMove();
        // getters
        Bitboard getBoardForColor(Color c);
        Color getSideToMove();
        // helper functions
        void putPiece(Square sq, Piece p);
        void putPiece(Square sq, PieceType pt, Color c);
        void silentDoMove(Move m);
        void moveCastleRook(Square from, Square to);
        void clear();
        string str();

        // pieces
        bool occupied(Square s);
        bool occupied(Square s, Color c);

        // attacks
        bool attacked(Square s, Color c);

        // rulebook
        bool legal(Move m);
        bool ownKingInCheckAfterMove(Move m);
        bool psudoLegal(Move m);
        bool psudoLegalPawn(Move m);
        bool psudoLegalKnight(Move m);
        bool psudoLegalBishop(Move m);
        bool psudoLegalRook(Move m);
        bool psudoLegalQueen(Move m);
        bool psudoLegalKing(Move m);
        // public fields
        Bitboard pieces[NUMBER_OF_COLORS-1][NUMBER_OF_PIECE_TYPES];
        Piece board[NUMBER_OF_SQUARES];
        StateInfo* stateInfo;

        Square enpassantTarget;
        CastlingRights castlingRights;
        /* protected: */
        vector<Move> moveList;
        Color sideToMove;


        unsigned halfmoveClock;
        unsigned fullmoveNumber;

};


inline Color Position::getSideToMove() {
    return sideToMove;
}

#endif
