#include <string>
#include <iostream>
#include <sstream>

#include "types.h"
#include "position.h"
#include "bitboards.h"
#include "pieces.h"

using std::cout;
using std::endl;

Position::Position() {
    clear();
}

void Position::setFromFEN(std::string fen) {
    clear();

    std::istringstream ss (fen);
    ss >> std::noskipws;
    Square sq = SQ_A8;
    unsigned char c;


    // 1. Place the pieces
    // as long as there are chars to be consumed and we are not at the end (when we hit a space)
    while ((ss >> c) && !isspace(c)) {
	if (c == 'w')
	    break;
	if (isdigit(c)) {
	    sq += Square(c - '0'); // - '0' converts char to int, advance c files
	} else if (c == '/') {
	    sq -= Square(16); // if c == '/', then sq is on file h. Go back 2*8, back to file A with rank one less
	} else {
	    putPiece(sq, Pieces::makePieceType(c), Pieces::makeColor(c));
	    ++sq;
	}
    }

    // 2. Set side to move
    ss >> c; // consume the char giving what side should move next. Is either w or b
    sideToMove = c == 'w' ? WHITE : BLACK;
    ss >> c; // consume trailing space

    // 3. Castle rights
    castlingRights = NO_CASTLING;
    while ((ss >> c) && !isspace(c)) {
	if (c == 'K')
	    castlingRights = (CastlingRights) (castlingRights | WHITE_OO);
	else if (c == 'Q')
	    castlingRights = (CastlingRights) (castlingRights | WHITE_OOO);
	else if (c == 'k')
	    castlingRights = (CastlingRights) (castlingRights | BLACK_OO);
	else if (c == 'q')
	    castlingRights = (CastlingRights) (castlingRights | BLACK_OOO);
    }
    
    // 4. En passant target square
    ss >> c; 
    if (c == '-')
	enpassantTarget = NO_SQUARE;
    else {
	File enpassantFile = makeFile(c);
	ss >> c;
	Rank enpassantRank = makeRank(c);
	enpassantTarget = Square(8*enpassantRank + enpassantFile);
    }
    
    ss >> c;


    // 5. halfmove clock
    ss >> c;
    string halfmoveString (1,c);
    while ((ss >> c) && !isspace(c)) {halfmoveString += c;}
    halfmoveClock = std::stoi(halfmoveString);

    // 6. fullmove number
    ss >> c;
    string fullmoveString (1,c);
    while ((ss >> c) && !isspace(c)) {fullmoveString += c;}
    fullmoveNumber = std::stoi(fullmoveString);
}

Bitboard Position::getBoardForColor(Color c) {
    return pieces[c][PAWN] | pieces[c][KNIGHT] | pieces[c][BISHOP] | pieces[c][ROOK] | pieces[c][QUEEN] | pieces[c][KING];
}


void Position::clear() {
    for (int r = RANK_1; r <= RANK_8; ++r) {
    	for (int f = FILE_A; f <= FILE_H; ++f) {
    	    board[8*r+f] = NO_PIECE;
    	}
    }
    for (int c = WHITE; c <= BLACK; ++c) {
	for (int pt = PAWN; pt <= KING; ++pt) {
	    pieces[c][pt] = BITBOARD_EMPTY;
	}
	pieces[c][NO_PIECE_TYPE] = BITBOARD_UNIVERSE;
    }
    
}

void Position::putPiece(Square sq, PieceType pt, Color c) {
    board[sq] = makePiece(c, pt);
    pieces[c][pt] |= (1ULL << sq); // set the piece in its right Bitboard
    // make sure there is no other pieces on sq
    for (int c2 = WHITE; c2 <= BLACK; ++c2) {
    	for (int pt2 = NO_PIECE_TYPE; pt2 < NUMBER_OF_PIECE_TYPES; ++pt2 ) {
    	    if (!(c == c2 && pt == pt2)) {
    		pieces[c2][pt2] &= (BITBOARD_UNIVERSE ^ (1ULL << sq));
    	    }
    	}
    }
}

void Position::putPiece(Square sq, Piece p) {
    putPiece(sq, makePieceType(p), makeColor(p));
}


void Position::doMove(Move m) {
    Square from = m.getFrom(), to = m.getTo();
    Piece p = board[from];

    // update to a new stateInfo
    StateInfo newStateInfo, *newInfo = &newStateInfo;
    newInfo->lastMove_originPiece = p;
    newInfo->lastMove_destinationPiece = board[to];
    newInfo->previous = stateInfo;
    stateInfo = newInfo;

    // place the piece
    putPiece(to, makePieceType(p), makeColor(p));
    putPiece(from, NO_PIECE_TYPE, NO_COLOR);

    // update fields
    sideToMove = colorSwap(sideToMove);
    fullmoveNumber += sideToMove == WHITE ? 1 : 0;
    if (!m.capture()) {
	if (makePieceType(board[m.getFrom()]) != PAWN)
	    halfmoveClock += 1;
	else
	    halfmoveClock = 0;
    } else 
	halfmoveClock = 0;
    moveList.push_back(m);
}

void Position::undoMove() {
    Move lastMove = moveList.back();
    moveList.pop_back();
    putPiece(lastMove.getFrom(), stateInfo->lastMove_originPiece);
    putPiece(lastMove.getTo(), stateInfo->lastMove_destinationPiece);
    stateInfo = stateInfo->previous;
}
