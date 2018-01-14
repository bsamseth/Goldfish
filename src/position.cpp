#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <random>

#include "types.h"
#include "position.h"
#include "bitboards.h"
#include "stateinfo.h"
#include "evaluate.h"

using Bitboards::prettyString;
using std::string;
using std::cout;
using std::endl;
using std::max;
using std::log2;

namespace Zobrist {
    Key psq[NUMBER_OF_PIECES][NUMBER_OF_SQUARES];
    Key enpassant[NUMBER_OF_FILES];
    Key kingpassant[NUMBER_OF_FILES];
    Key castling[NUMBER_OF_CASTLING_RIGHTS];
    Key side;
}

void Position::initHashing() {
    // Seed with first 8 digits of the SHA256 hexdigest of Goldfish.
    std::mt19937_64 rng(0xc35adbe1e);
    // Initialize th random numbers.
    for (int p = NO_PIECE; p < NUMBER_OF_PIECES; p++)
        for (Square s = SQ_A1; s <= SQ_H8; ++s)
            Zobrist::psq[p][s] = rng();
    for (File f = FILE_A; f <= FILE_H; ++f) {
        Zobrist::enpassant[f] = rng();
        Zobrist::kingpassant[f] = rng();
    }
    for (int cr = NO_CASTLING; cr <= ANY_CASTLING; ++cr)
        Zobrist::castling[cr] = rng();
    Zobrist::side = rng();

    // Initialize hash
    hash = 0;
    for (Square s = SQ_A1; s <= SQ_H8; ++s)
        hash ^= Zobrist::psq[board[s]][s];
    if (enpassantTarget != NO_SQUARE)
        hash ^= Zobrist::enpassant[makeFile(enpassantTarget)];
    if (kingpassantTarget != NO_SQUARE)
        hash ^= Zobrist::kingpassant[makeFile(kingpassantTarget)];
    hash ^= Zobrist::castling[castlingRights];
    hash ^= sideToMove == WHITE ? Zobrist::side : 0;
}

Position::Position() {
    clear();
    stateInfo = rootState; // root state, zero init
    setFromFEN(STARTING_FEN);
    initHashing();
}

Position::Position(std::string fen) {
    clear();
    stateInfo = rootState;
    setFromFEN(fen);
    initHashing();
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
        File enpassant = makeFile(c);
        ss >> c;
        Rank enpassantRank = makeRank(c);
        enpassantTarget = Square(8*enpassantRank + enpassant);
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

Bitboard Position::getBoardForColor(Color c) const {
    return pieces[c][PAWN] | pieces[c][KNIGHT] | pieces[c][BISHOP] | pieces[c][ROOK] | pieces[c][QUEEN] | pieces[c][KING];
}


void Position::clear() {
    kingpassantTarget = enpassantTarget = NO_SQUARE;
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
    st->lastMove_kingpassantTarget = kingpassantTarget;
    st->previous_castlingRights = castlingRights;
    st->lastHash = hash;
    st->previous = stateInfo;
    stateInfo = st;

    // place the piece
    hash ^= Zobrist::psq[board[from]][from] ^ Zobrist::psq[board[to]][to];
    hash ^= Zobrist::psq[NO_PIECE][from] ^ Zobrist::psq[board[from]][to];
    hash ^= Zobrist::side;
    hash ^= enpassantTarget != NO_SQUARE ? Zobrist::enpassant[makeFile(enpassantTarget)]
                                         : 0;
    hash ^= kingpassantTarget != NO_SQUARE ? Zobrist::kingpassant[makeFile(kingpassantTarget)]
                                         : 0;
    silentDoMove(m);


    // update fields
    sideToMove = colorSwap(sideToMove);
    fullmoveNumber += sideToMove == WHITE ? 1 : 0;
    halfmoveClock += (!m.capture() and makePieceType(p) != PAWN) ? 1 : 0;
    enpassantTarget = m.doublePawnPush() ? Square((to + from)/2) : NO_SQUARE;

    // castlingrights changed?
    const CastlingRights old = castlingRights;
    if ((castlingRights & WHITE_OO) || (castlingRights & WHITE_OOO)) {
        if (p == W_KING)
            castlingRights = CastlingRights( castlingRights ^ (WHITE_OO | WHITE_OOO) );
        else if (from == SQ_H1)
            castlingRights = CastlingRights( castlingRights & ~(WHITE_OO) );
        else if (from == SQ_A1)
            castlingRights = CastlingRights( castlingRights & ~(WHITE_OOO) );
    }
    else if ((castlingRights & BLACK_OO) || (castlingRights & BLACK_OOO)) {
        if (p == B_KING)
            castlingRights = CastlingRights( castlingRights & ~(BLACK_OO | BLACK_OOO) );
        else if (from == SQ_H8)
            castlingRights = CastlingRights( castlingRights & ~(BLACK_OO) );
        else if (from == SQ_A8)
            castlingRights = CastlingRights( castlingRights & ~(BLACK_OOO) );
    }
    if (castlingRights != old)
        hash ^= Zobrist::castling[old] ^ Zobrist::castling[castlingRights];

    // castle move?
    if (m.castle())
        moveCastleRook(from, to); // Also sets kingpassant.
    else
        kingpassantTarget = NO_SQUARE;

    // Promotion?
    if (m.promotion()) {
        hash ^= Zobrist::psq[board[to]][to];
        board[to] = makePiece(makeColor(p), move_to_promotion_piece[m.getFlag()]);
        hash ^= Zobrist::psq[board[to]][to];
    }

    // Set hash for passant targets.
    hash ^= enpassantTarget != NO_SQUARE ? Zobrist::enpassant[makeFile(enpassantTarget)]
                                         : 0;
    hash ^= kingpassantTarget != NO_SQUARE ? Zobrist::kingpassant[makeFile(kingpassantTarget)]
                                         : 0;

    // Add move to move list.
    moveList.push_back(m);
}

/*
 * Helper function for doMove. Assumes that Move(from, to) is
 * a valid castling move, and move the corresponding rook. Also
 * sets the kingpassant square accordingly.
 */
void Position::moveCastleRook(Square from, Square to) {
    if (from == SQ_E1) {
        if (to == SQ_G1) {
            kingpassantTarget = SQ_F1;
            silentDoMove(Move(SQ_H1, SQ_F1));
        }
        else if (to == SQ_C1) {
            kingpassantTarget = SQ_D1;
            silentDoMove(Move(SQ_A1, SQ_D1));
        }
    } else if (from == SQ_E8) {
        if (to == SQ_G8) {
            kingpassantTarget = SQ_F8;
            silentDoMove(Move(SQ_H8, SQ_F8));
        }
        else if (to == SQ_C1) {
            kingpassantTarget = SQ_D8;
            silentDoMove(Move(SQ_A8, SQ_D8));
        }
    }
}

/*
 * Helper. Only does the move and places a NO_PIECE on
 * the origin square. Nothing else is done to fields.
 * A call to this is not reversible as destination piece is lost.
 */
void Position::silentDoMove(Move m) {
    Piece p = board[m.getFrom()];
    putPiece(m.getTo(), makePieceType(p), makeColor(p));
    putPiece(m.getFrom(), NO_PIECE_TYPE, NO_COLOR);
}

void Position::undoMove() {
    Move lastMove = moveList.back();
    moveList.pop_back();
    sideToMove = colorSwap(sideToMove);
    fullmoveNumber = stateInfo->previous_fullmoveNumber;
    halfmoveClock = stateInfo->previous_halfmoveClock;
    castlingRights = stateInfo->previous_castlingRights;
    enpassantTarget = stateInfo->lastMove_enpassantTarget;
    kingpassantTarget = stateInfo->lastMove_kingpassantTarget;
    putPiece(lastMove.getFrom(), stateInfo->lastMove_originPiece);
    putPiece(lastMove.getTo(), stateInfo->lastMove_destinationPiece);
    hash = stateInfo->lastHash;
    StateInfo* &ptr = stateInfo->previous;
    delete stateInfo;
    stateInfo = ptr;

    //castle move? replace rook
    if (lastMove.castle()) {
        Square to = lastMove.getTo();
        if (to == SQ_G1)
            silentDoMove(Move(SQ_F1, SQ_H1));
        else if (to == SQ_C1)
            silentDoMove(Move(SQ_D1, SQ_A1));
        else if (to == SQ_G8)
            silentDoMove(Move(SQ_F8, SQ_H8));
        else if (to == SQ_C8)
            silentDoMove(Move(SQ_D8, SQ_A8));
    }
}

int Position::score() const {
    int s = 0;
    for (int r = RANK_1; r <= RANK_8; ++r) {
        for (int f = FILE_A; f <= FILE_H; ++f) {
            Square curr = Square(8*r+f);
            int value = piece_value(board[curr], curr);
            s += value;
        }
    }
    return s;
}

bool Position::occupied(Square s) const {
    return occupied(s, WHITE) || occupied(s, BLACK);
}

bool Position::occupied(Square s, Color c) const {
    bool occupied1 = (board[s] != NO_PIECE) && (makeColor(board[s]) == c);
    // bool occupied2 = (pieces[c][NO_PIECE_TYPE] & (1ULL << s)) == 0;
    // assert(occupied1 == occupied2);
    // not the case, pieces[BLACK][NO_PIECE_TYPE] = pieces[WHITE][NO_PIECE_TYPE]
    return occupied1;
}

string Position::str() const {
    std::stringstream sstm;
    for (int i = 7; i >= 0; i--) {
        for (int j = 0; j < 8; j++) {
            sstm << "|_" << (board[8*i + j] == NO_PIECE ? "_" : PieceName[board[8*i + j]]) << "_";
        }
        sstm << "|\n";
    }
    return sstm.str().substr(0,sstm.str().size() -1);
}
