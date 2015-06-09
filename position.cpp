#include <string>
#include <iostream>
#include <sstream>
#include <cassert>

#include "types.h"
#include "position.h"
#include "bitboards.h"
#include "pieces.h"

using Bitboards::prettyString;
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
    if (pt != NO_PIECE_TYPE)
	pieces[c][NO_PIECE_TYPE] &= ~(1ULL << sq);
    
    // make sure there is no other pieces on sq
    for (int c2 = WHITE; c2 <= BLACK; ++c2) {
    	for (int pt2 = PAWN; pt2 < NUMBER_OF_PIECE_TYPES; ++pt2 ) {
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
    putPiece(from, NO_PIECE_TYPE, makeColor(p));
    // putPiece(from, NO_PIECE_TYPE, colorSwap(makeColor(p)));

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
    
    // set en passant square
    if (m.doublePawnPush())
		enpassantTarget = Square((to + from)/2); // taget is on square between from and to, take the average
    else 
		enpassantTarget = NO_SQUARE;
    
    moveList.push_back(m);
}

void Position::undoMove() {
    Move lastMove = moveList.back();
    moveList.pop_back();
    putPiece(lastMove.getFrom(), stateInfo->lastMove_originPiece);
    putPiece(lastMove.getTo(), stateInfo->lastMove_destinationPiece);
    stateInfo = stateInfo->previous;
}


bool Position::occupied(Square s) {
    // assert ( ~getBoardForColor(WHITE) == pieces[WHITE][NO_PIECE_TYPE] );
    // assert ( ~getBoardForColor(BLACK) == pieces[BLACK][NO_PIECE_TYPE] );

    bool occupied1 = board[s] != NO_PIECE;
    bool occupied2 = (pieces[WHITE][NO_PIECE_TYPE] & (1ULL << s)) == 0 || (pieces[BLACK][NO_PIECE_TYPE] & (1ULL << s)) == 0;
    // cout << "occupied1 = " << occupied1 << ", occupied2 = " << occupied2 << endl;
    
    // cout << "White no-piece board:\n" << prettyString(pieces[WHITE][NO_PIECE_TYPE]);// | getBoardForColor(BLACK)) << endl;;
    // cout << "Black no-piece board:\n" << prettyString(pieces[BLACK][NO_PIECE_TYPE]);// | getBoardForColor(BLACK)) << endl;;
    assert(occupied1 == occupied2);
    return occupied1;
}

bool Position::occupied(Square s, Color c) {
    bool occupied1 = board[s] != NO_PIECE && makeColor(board[s]) == c;
    bool occupied2 = (pieces[c][NO_PIECE_TYPE] & (1ULL << s)) == 0;
    assert(occupied1 == occupied2);
    return occupied1;
}

bool Position::psudoLegal(Move m) {
    Square s1 = m.getFrom(), s2 = m.getTo();
    assert ( occupied(s1) ); // m is assumed to have a piece on its origin square

    Piece p = board[s1]; // the piece on s1
    Color us = makeColor(p);
    PieceType pt = makePieceType(p);

    // if piece is wrong color or friendly piece on s2, then no good
    if (us != sideToMove || us == makeColor(board[s2]))
	return false;

    switch (makePieceType(p)) {
    case PAWN:   return psudoLegalPawn(m,s1,s2,p,us,pt);
    case KNIGHT: return psudoLegalKnight(m,s1,s2,p,us,pt);
    case BISHOP: return psudoLegalBishop(m,s1,s2,p,us,pt);
    case ROOK:   return psudoLegalRook(m,s1,s2,p,us,pt);
    case QUEEN:  return psudoLegalQueen(m,s1,s2,p,us,pt);
    case KING:   return psudoLegalKing(m,s1,s2,p,us,pt);
    default:     assert (("In psudoLegal, no match for piece type.", false));
    }
}

bool Postion::psudoLegalPawn(Move m, Square s1, Square s2, Piece p, Color us, PieceType pt) {
    // pawns are set in one direction, code is from whites perspective
    Square up    = us == WHITE ? D_NORTH : D_SOUTH;
    Square tl    = us == WHITE ? D_NORTH_WEST : D_SOUTH_EAST; // take_left
    Square tr    = us == WHITE ? D_NORTH_EAST : D_SOUTH_WEST; // take_right
    Bitboard r2  = us == WHITE ? RANK_2_BB : RANK_7_BB;
    Bitboard r7  = us == WHITE ? RANK_7_BB : RANK_2_BB;
    Square diff  = s2 - s1; // positve for white, negative for black (if legal)


    // handle en passant first
    if (m.ep_capture()) {
	// this should be predefined as a valid psudo legal move, but just in case
	assert (("Move is en passant, is also capture.", m.capture()))
	if (diff != tl && diff != tr) // diff is neither tl nor tr, false
	    assert (("move encoded as en passant, but not a valid capture move..", false));
	    return false;
	else 
	    return true;
    }

    // promotions
    if (m.promotion()) {
	assert (("If the pawn move is a promotion, should be on 7'th rank.", (bool) (r7 & (1 << s1))));
	if (m.capture()) {
	    assert (("Promo-capture, diff must be tl or tr", diff == tl || diff == tr));
	}
	// if all assertions are passed, than this is legal
	return true;
    }

    if (m.capture()) { // move is a normal capture
	assert (("Pawn move to a enemy occupied square. Should be a capture.", occupied(s2, colorSwap(us))));
	assert (("Pawn move encoded as capture, so it should be psudoLegal.", diff == tl || diff == tr));
	return (diff == tl || diff == tr);
    }

    // remainig legal moves are pushes
    if (m.doublePawnPushMove()) {
	assert (("Pawn move encoded as double push, diff should match", diff == (up+up)));
	return diff == (up+up);
    }
    // at this point the only legal move is a simple push, and there is no piece on s2
    return diff == up;
}
