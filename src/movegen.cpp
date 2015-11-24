#include <iostream>
#include <string>

#include "types.h"
#include "movegen.h"
#include "move.h"
#include "position.h"


using std::string;
using std::cout;
using std::endl;


MoveGenerator::MoveGenerator(const Position& position) {
    pos = &position;
}

void MoveGenerator::generateMoves() {
    Square cSquare;

    for (Rank rank = RANK_1; rank <= RANK_8) {
	for (File file = FILE_A; file <= FILE_H; file++) {
	    cSquare = Square(rank*8 + file);
	    if (!pos->occupied(cSquare, pos->getSideToMove()))
		continue;
	    
	}
    }
}

// encode the suggested move, and then add it to the generatedMoves vector.
// It assumes that pos->psudoLegal(s1, s2) has returned true.
void MoveGenerator::encodeAndAddMove(Square s1, Square s2) {
    Piece p1 = pos->board[s1], p2 = pos->board[s2];
    MoveFlag flag = QUIET_MOVE;
    Square up = makeColor(p1) == WHITE ? D_NORTH : D_SOUTH;
    
    // en passant?
    if (pos->enpassantTarget != NO_SQUARE && p1 == PAWN && s2 == enpassantTarget) { // && is capture / is not on same file, TODO
	flag = ENPASSANT_CAPTURE_MOVE;
	return void;
    }
	
    // promotion?
    if (p1 == PAWN && (s2 & RANK_8_BB || s2 & RANK_8_BB) )  {
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
	return void;
    }

    // capture?
    if (p2 != NO_PIECE) {
	flag |= CAPTURE_MOVE;
	return void;
    }

    
    // double pawn push?
    if (p1 == PAWN && (int(to) - int(from) == 2*int(up))) {
	generatedMoves.push_back(Move(from, to, DOUBLE_PAWN_PUSH_MOVE));
	return void;
    }
    
}



