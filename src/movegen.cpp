#include <iostream>
#include <string>

#include "types.h"
#include "bitboards.h"
#include "movegen.h"
#include "move.h"
#include "position.h"


using std::string;
using std::cout;
using std::endl;

/*
 * Initialize the generator with a position.
 */
MoveGenerator::MoveGenerator(Position& position) {
  pos = &position;
}

/*
 * After a call the field std::vec<Move> generatedmoves
 * will be filled with all the possible moves from the
 * current position. All moves will be encoded with a
 * proper flag.
 */
void MoveGenerator::generateMoves() {
  Square cSquare, target;
    
  for (int rank = RANK_1; rank <= RANK_8; rank++) {
    for (int file = FILE_A; file <= FILE_H; file++) {

      cSquare = Square(rank*8 + file);
      if (!pos->occupied(cSquare, pos->getSideToMove()))
	continue;

      for (int rank2 = RANK_1; rank2 <= RANK_8; rank2++) {
	for (int file2 = FILE_A; file2 <= FILE_H; file2++) {
	  target = Square(rank2*8 + file2);
	  if (cSquare != target && pos->legal(Move(cSquare, target)))
	    // cout << "found a move : " << Move(cSquare, target).str() << endl;
	    encodeAndAddMove(cSquare, target);
	}
      }
    }
  }
}

/*
 * Encode the suggested move, and then add it to the generatedMoves vector.
 * It assumes that pos->psudoLegal(s1, s2) has returned true.
 */
void MoveGenerator::encodeAndAddMove(Square s1, Square s2) {
  Piece p1 = pos->board[s1], p2 = pos->board[s2];
  Square up = makeColor(p1) == WHITE ? D_NORTH : D_SOUTH;
  Square from = s1;
  Square to   = s2;
    
  // en passant?
  if (pos->enpassantTarget != NO_SQUARE && makePieceType(p1) == PAWN && s2 == pos->enpassantTarget) { 
    generatedMoves.push_back(Move(from, to, ENPASSANT_CAPTURE_MOVE));
    return;
  }
	
  // promotion?
  if (makePieceType(p1) == PAWN && (s2 & RANK_8_BB || s2 & RANK_8_BB) )  {
    // generate all the possible promotions
    if (p2 != NO_PIECE) { // promo capture?
      generatedMoves.push_back(Move(from, to, KNIGHT_PROMO_CAPTURE_MOVE));
      generatedMoves.push_back(Move(from, to, BISHOP_PROMO_CAPTURE_MOVE));
      generatedMoves.push_back(Move(from, to, ROOK_PROMO_CAPTURE_MOVE));
      generatedMoves.push_back(Move(from, to, QUEEN_PROMO_CAPTURE_MOVE));
    } else {
      generatedMoves.push_back(Move(from, to, KNIGHT_PROMO_MOVE));
      generatedMoves.push_back(Move(from, to, BISHOP_PROMO_MOVE));
      generatedMoves.push_back(Move(from, to, ROOK_PROMO_MOVE));
      generatedMoves.push_back(Move(from, to, QUEEN_PROMO_MOVE));
    }
    return;
  }

  // capture?
  if (p2 != NO_PIECE) {
    generatedMoves.push_back(Move(from, to, CAPTURE_MOVE));
    return;
  }

    
  // double pawn push?
  if (makePieceType(p1) == PAWN && (int(to) - int(from) == 2*int(up))) {
    generatedMoves.push_back(Move(from, to, DOUBLE_PAWN_PUSH_MOVE));
    return;
  }

  generatedMoves.push_back(Move(from, to, QUIET_MOVE));
}



