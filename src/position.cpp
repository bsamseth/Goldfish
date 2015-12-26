#include <string>
#include <iostream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <cmath>

#include "types.h"
#include "position.h"
#include "bitboards.h"
#include "stateinfo.h"

using Bitboards::prettyString;
using std::string;
using std::cout;
using std::endl;
using std::max;
using std::log2;

Position::Position() {
  clear();
  stateInfo = rootState; // root state, zero init
}

Position::Position(std::string fen) {
  clear();
  stateInfo = rootState;
  setFromFEN(fen);
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
      putPiece(sq, makePieceTypeFromChar(c), makeColorFromChar(c));
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
  assert ( int(pt) < NUMBER_OF_PIECE_TYPES ); // unknown piece type
  assert ( int(c) < NUMBER_OF_COLORS ); // unknown piece type
  
  board[sq] = makePiece(c, pt);
  if (pt != NO_PIECE_TYPE) {
    pieces[c][pt] |= (1ULL << sq); // set the piece in its right Bitboard
    pieces[WHITE][NO_PIECE_TYPE] &= ~(1ULL << sq);
    pieces[BLACK][NO_PIECE_TYPE] &= ~(1ULL << sq);
    
  } else {
    pieces[WHITE][NO_PIECE_TYPE] |= (1ULL << sq);
    pieces[BLACK][NO_PIECE_TYPE] |= (1ULL << sq);
    // make sure there is no other pieces on sq
    for (int c2 = WHITE; c2 <= BLACK; ++c2) {
      for (int pt2 = PAWN; pt2 < NUMBER_OF_PIECE_TYPES; ++pt2 ) {
	pieces[c2][pt2] &= ~(1ULL << sq);
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

  // update stateinfo
  StateInfo* st = new StateInfo();
  st->lastMove_originPiece = p;
  st->lastMove_destinationPiece = board[to];
  st->previous_halfmoveClock = halfmoveClock;
  st->previous_fullmoveNumber = fullmoveNumber;
  st->lastMove_enpassantTarget = enpassantTarget;
  st->previous = stateInfo;
  stateInfo = st;

  // place the piece
  putPiece(to, makePieceType(p), makeColor(p));
  putPiece(from, NO_PIECE_TYPE, NO_COLOR);
  // putPiece(from, NO_PIECE_TYPE, colorSwap(makeColor(p)));

  // update fields
  sideToMove = colorSwap(sideToMove);
  fullmoveNumber += sideToMove == WHITE ? 1 : 0;
  if (!m.capture()) {
    if (makePieceType(p) != PAWN)
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
  StateInfo* &ptr = stateInfo->previous;
  delete stateInfo;
  stateInfo = ptr->previous;

  // update fields
  sideToMove = colorSwap(sideToMove);
  fullmoveNumber -= sideToMove == BLACK ? 1 : 0;
  if (!lastMove.capture()) {
    if (makePieceType(board[lastMove.getFrom()]) != PAWN)
      halfmoveClock -= 1;
    else
      halfmoveClock = 0;
  } else 
    halfmoveClock = 0;
    
  // set en passant square
  Move previous_move = moveList.back();
  if (previous_move.doublePawnPush())
    enpassantTarget = Square((previous_move.getTo() + previous_move.getFrom())/2); // taget is on square between from and to, take the average
  else 
    enpassantTarget = NO_SQUARE;
}


bool Position::occupied(Square s) {
  return occupied(s, WHITE) || occupied(s,BLACK);
}

bool Position::occupied(Square s, Color c) {
  bool occupied1 = board[s] != NO_PIECE && makeColor(board[s]) == c;
  // bool occupied2 = (pieces[c][NO_PIECE_TYPE] & (1ULL << s)) == 0;
  // assert(occupied1 == occupied2);
  // not the case, pieces[BLACK][NO_PIECE_TYPE] = pieces[WHITE][NO_PIECE_TYPE]
  return occupied1;
}

/*
 * Returns true if the move is a psudo legal move,
 * and the moving sides king is not in check after
 * the move has been made.
 */
bool Position::legal(Move m) {
  return psudoLegal(m) && !ownKingInCheckAfterMove(m);
}

/*
 * Return true if the moving sides king is in check after the
 * move has been made. 
 */
bool Position::ownKingInCheckAfterMove(Move m) {
  bool inCheck = false;
  Square kingSquare = Square(log2(pieces[sideToMove][KING]));
  Square cSquare;
  doMove(m);
  
  for (int rank = RANK_1; rank <= RANK_8; rank+=1) {
    for (int file = FILE_A; file <= FILE_H; file+=1) {
      cSquare = Square(8*rank + file);
      if (occupied(cSquare, sideToMove)) {
	if (cSquare != kingSquare && psudoLegal(Move(cSquare, kingSquare))) {
	  inCheck = true;
	  break;
	}
      }
    }
    if (inCheck)
      break;
  }
  undoMove();
  return inCheck;
}

/*
 * Returns true if the move proposed is psudo legal.
 * This means that it follows the move rules for the piece
 * in question. In perticular, this returns false if the destination
 * square is occupied by a friendly.
 *
 * A legal move is a psudo legal move that does not leave
 * own king in check after the move has been made.
 *
 * Assumptions (asserted):
 * - target square is different from origin
 * - origin square is occupied
 * - color of piece on origin is same as sidetomove
 * - target is not a non-square
 *
 * The function does not assume any move encoding has been done.
 * This might be an option to change.
 */
bool Position::psudoLegal(Move m) {
  Square s1 = m.getFrom(), s2 = m.getTo();
  Piece p = board[s1]; // the piece on s1
  Color us = makeColor(p);
  PieceType pt = makePieceType(p);

  assert ( s1 != s2); // this should be a move, not a null move
  assert ( occupied(s1) ); // m is assumed to have a piece on its origin square
  assert ( s2 != NO_SQUARE ); // cannot move to a non-square!
  assert (us == sideToMove); // if not, function is called unnecessary by movegen

  // if friendly piece on s2, then no good
  if ( us == makeColor(board[s2]))
    return false;

  switch (pt) {
  case PAWN:   return psudoLegalPawn(m);
  case KNIGHT: return psudoLegalKnight(m);
  case BISHOP: return psudoLegalBishop(m);
  case ROOK:   return psudoLegalRook(m);
  case QUEEN:  return psudoLegalQueen(m);
  case KING:   return psudoLegalKing(m);
  default:     assert ((cout << "In psudoLegal, no match for piece type: " << pt << endl, false));
  }
}


/*
 * Returns true if s1-s2 is a psudo legal pawn move.
 * En passant handled first, then captures followd by
 * pushes. Promotions not given any extra attention.
 */
bool Position::psudoLegalPawn(Move m) {
  Square s1 = m.getFrom(), s2 = m.getTo();
  Piece p = board[s1]; // the piece on s1
  Color us = makeColor(p);
  // pawns are set in one direction, code is from whites perspective
  Square up    = us == WHITE ? D_NORTH : D_SOUTH;
  Square tl    = us == WHITE ? D_NORTH_WEST : D_SOUTH_EAST; // take_left
  Square tr    = us == WHITE ? D_NORTH_EAST : D_SOUTH_WEST; // take_right
  Bitboard r2  = us == WHITE ? RANK_2_BB : RANK_7_BB;
  // Bitboard r7  = us == WHITE ? RANK_7_BB : RANK_2_BB;
  // Square diff  = s2 - s1; // positve for white, negative for black (if legal)


  // handle en passant first
  // needs to be capture, and land on enpassantTarget
  // this relies on enpassantTarget to be set corretly
  if ( (s2 == s1 + tl || s2 == s1 + tr) && s2 == enpassantTarget) 
    return true;
    
  // promotions should not be considered here, need only know if the pawn is allowed
  // move is a normal capture

  // is capture 
  if  ((s2 == s1 + tl || s2 == s1 + tr) && occupied(s2, colorSwap(us)))
    return true;

  // is double push
  if ( ( !occupied((s1+up)+up) && s2 == (s1+up) + up ) && ((1 << s1) & r2) ) 
    return true;

  // is single push
  if ( !occupied(s2) && s2 == s1+up)
    return true;

  // if we get here the move is not psudolegal
  return false;
}

/*
 * Returns true if s1-s2 is a psudo legal knight move.
 * This is the case if the move changes either one file and
 * two ranks, or two files and one rank.
 * Done by seting total rank/file-diff to 3, and requiring
 * that none of the diffs are greater than 2.
 */
bool Position::psudoLegalKnight(Move m) {
  Square s1 = m.getFrom(), s2 = m.getTo();
  int d_rank = abs(rank_diff(s1, s2));
  int d_file = abs(file_diff(s1, s2));
  
  return (d_rank + d_file == 3 && d_rank < 3 && d_file < 3);
}

/*
 * Returns true if s1-s2 is a psudo legal bishop move.
 * This is the case if the move changes rank and file
 * the same amount, and no piece is blocking the way.
 */
bool Position::psudoLegalBishop(Move m) {
  Square s1 = m.getFrom(), s2 = m.getTo();
  int d_rank = rank_diff(s1, s2);
  int d_file = file_diff(s1, s2);
  if (abs(d_rank) != abs(d_file)) return false;

  int rank_sign = d_rank < 0 ? -1 : 1;
  int file_sign = d_file < 0 ? -1 : 1;
  for (int i = 1; i < d_file; i++) {
    if (occupied(Square(s1 + i*D_EAST*file_sign + i*D_NORTH*rank_sign))) {
      return false;
    }
  }
  return true;
}

/*
 * Returns true if s1-s2 is a psudo legal rook move.
 * This is the case if the move either does not change
 * rank, or does not change file, and no piece is in
 * the way.
 */
bool Position::psudoLegalRook(Move m) {
  Square s1 = m.getFrom(), s2 = m.getTo();
  int d_rank = rank_diff(s1, s2);
  int d_file = file_diff(s1, s2);
  if (d_rank != 0 && d_file != 0) return false;

  // test for blocking pieces
  Square direction;
  int dist = max(abs(d_file), abs(d_rank));
  if ( d_rank == 0 )
    direction = d_file < 0 ? D_WEST : D_EAST;
  else
    direction = d_rank < 0 ? D_SOUTH : D_NORTH;

  for (int i = 1; i < dist; i++) {
    if (occupied(Square(s1 + i*direction)))
      return false;
  }
  
  return true;
}


/*
 * Returns true if s1-s2 is a psudo legal rook move.
 * This is the case if the move is either a legal bishop
 * move, or a legal rook move.
 */
bool Position::psudoLegalQueen(Move m) {
  return psudoLegalBishop(m) || psudoLegalRook(m);
}


/*
 * Returns true if s1-s2 is a psudo legal king move.
 * This is the case if the move changes either one rank,
 * or one file or both.
 * This is tested by requiring the total rank/file diff to be
 * less than 3, while none of them can be bigger than 1. 
 */
bool Position::psudoLegalKing(Move m) {
  Square s1 = m.getFrom(), s2 = m.getTo();
  int d_rank = abs(rank_diff(s1, s2));
  int d_file = abs(file_diff(s1, s2));

  return (d_rank + d_rank < 3 && d_rank < 2 && d_file < 2);
}


string Position::str() {
  std::stringstream sstm;
  // sstm << "|_";

  for (int i = 7; i >= 0; i--) {
    for (int j = 0; j < 8; j++) {
      sstm << "|_" << (board[8*i + j] == NO_PIECE ? "_" : PieceName[board[8*i + j]]) << "_";
    }
    sstm << "|\n";
  } 
  return sstm.str().substr(0,sstm.str().size() -1);
}
