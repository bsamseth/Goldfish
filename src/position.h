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
  Color getSideToMove();
  // helper functions
  void putPiece(Square sq, Piece p);
  void putPiece(Square sq, PieceType pt, Color c);
  void clear();
    
  // pieces
  bool occupied(Square s);
  bool occupied(Square s, Color c);

  // rulebook
  bool legal(Move m);
  bool ownKingInCheckAfterMove(Move m);
  bool psudoLegal(Move m);
  bool psudoLegalPawn(Move m, Square s1, Square s2, Piece p, Color us, PieceType pt);
  bool psudoLegalKnight(Move m, Square s1, Square s2, Piece p, Color us, PieceType pt);
  bool psudoLegalBishop(Move m, Square s1, Square s2, Piece p, Color us, PieceType pt);
  bool psudoLegalRook(Move m, Square s1, Square s2, Piece p, Color us, PieceType pt);
  bool psudoLegalQueen(Move m, Square s1, Square s2, Piece p, Color us, PieceType pt);
  bool psudoLegalKing(Move m, Square s1, Square s2, Piece p, Color us, PieceType pt);
  // public fields
  Bitboard pieces[NUMBER_OF_COLORS][NUMBER_OF_PIECE_TYPES];
  Piece board[NUMBER_OF_SQUARES];
  StateInfo* stateInfo;

  Square enpassantTarget;
  protected:
  vector<Move> moveList;
  Color sideToMove;
  CastlingRights castlingRights;

  unsigned halfmoveClock;
  unsigned fullmoveNumber;

};


inline Color Position::getSideToMove() {
  return sideToMove;
}

#endif
