
#ifndef TYPES_H
#define TYPES_H

#include <cctype>
#include <cstdlib>
#include <string>
#include <iostream>
#include <numeric> // for std::iota
using Bitboard = uint64_t;

namespace Bitboards {
    std::string prettyString(Bitboard b);
}


enum CastlingSide {
    KING_SIDE, QUEEN_SIDE
};

/*
 * The different castling rights are represented by the first 4 bits
 * E.g. 000..0001 is WHITE_OO, and 000...0100 is BLACK_OO.
 * ANY_CASTLING is 000...001111, meaning all of the above.
 */
enum CastlingRights {
    NO_CASTLING = 0,
    WHITE_OO  = 1, 
    WHITE_OOO = WHITE_OO << 1,
    BLACK_OO  = WHITE_OO << 2,
    BLACK_OOO = WHITE_OO << 3, 
    ANY_CASTLING = WHITE_OO | WHITE_OOO | BLACK_OO | BLACK_OOO
};

/*
 * A1 = 0, B1 = 1, ... , H8 = 63
 */
enum Square {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    
    NUMBER_OF_SQUARES = 64,

    D_NORTH = 8,
    D_WEST  = -1,
    D_EAST  = 1,
    D_SOUTH = -8,

    D_NORTH_EAST = D_NORTH + D_EAST,
    D_SOUTH_EAST = D_SOUTH + D_EAST,
    D_SOUTH_WEST = D_SOUTH + D_WEST,
    D_NORTH_WEST = D_NORTH + D_WEST,

    NO_SQUARE = 123
};



enum File {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

enum Rank {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
};


enum PieceType {
    NO_PIECE_TYPE = 0, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NUMBER_OF_PIECE_TYPES
};

enum Color {
    WHITE = 0, BLACK, NO_COLOR, NUMBER_OF_COLORS
};

enum Piece {
    NO_PIECE = 0,
    W_PAWN = (WHITE)*6 + PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = (BLACK)*6 + PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING
};

inline Color colorSwap(Color c) {
    return c == WHITE ? BLACK : WHITE;
}

inline Piece makePiece(Color c, PieceType pt) {
    return (pt == NO_PIECE_TYPE || c == NO_COLOR) ? NO_PIECE : Piece(c*6 + pt);
}
inline PieceType makePieceType(Piece p) {
    return PieceType(int(p) % 6);
}
inline Color makeColor(Piece p) {
    return p == NO_PIECE ? NO_COLOR : (int(p) < int(B_PAWN) ? WHITE : BLACK);
}




// this is copied from Stockfish, just define operations the logical way
#define ENABLE_BASE_OPERATORS_ON(T)					\
    inline T operator+(T d1, T d2) { return T(int(d1) + int(d2)); }	\
    inline T operator-(T d1, T d2) { return T(int(d1) - int(d2)); }	\
    inline T operator*(int i, T d) { return T(i * int(d)); }		\
    inline T operator*(T d, int i) { return T(int(d) * i); }		\
    inline T operator-(T d) { return T(-int(d)); }			\
    inline T& operator+=(T& d1, T d2) { return d1 = d1 + d2; }		\
    inline T& operator-=(T& d1, T d2) { return d1 = d1 - d2; }		\
    inline T& operator*=(T& d, int i) { return d = T(int(d) * i); }

#define ENABLE_FULL_OPERATORS_ON(T)					\
    ENABLE_BASE_OPERATORS_ON(T)						\
	inline T& operator++(T& d) { return d = T(int(d) + 1); }        \
    inline T& operator--(T& d) { return d = T(int(d) - 1); }		\
    inline T operator/(T d, int i) { return T(int(d) / i); }		\
    inline int operator/(T d1, T d2) { return int(d1) / int(d2); }	\
    inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

ENABLE_FULL_OPERATORS_ON(Square);
ENABLE_FULL_OPERATORS_ON(File);
ENABLE_FULL_OPERATORS_ON(Rank);
ENABLE_FULL_OPERATORS_ON(Color);
ENABLE_FULL_OPERATORS_ON(PieceType);

#undef ENABLE_FULL_OPERATORS_ON
#undef ENABLE_BASE_OPERATORS_ON





inline File makeFile(char f) {
    try {
    f = tolower(f);
    switch (f) {
    case 'a': return FILE_A; break;
    case 'b': return FILE_B; break;
    case 'c': return FILE_C; break;
    case 'd': return FILE_D; break;
    case 'e': return FILE_E; break;
    case 'f': return FILE_F; break;
    case 'g': return FILE_G; break;
    case 'h': return FILE_H; break;
    default: throw 101; 
    }
    } catch (int e) {
	std::cout << "Tried to make File from char " << f << ". Exiting..." << std::endl;
	std::exit(e);
    }
    return FILE_A;
}

inline Rank makeRank(char r) {
    return Rank(r - '1');
}


#endif
