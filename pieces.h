#ifndef PIECE_H
#define PIECE_H
#include <cctype>  // for tolower
#include <iostream>
#include <cstdlib> // for exit

#include "types.h"

namespace Pieces {

PieceType makePieceType(char c) {
    c = tolower(c);
    switch (c) {
    case 'p': return PAWN;
    case 'n': return KNIGHT;
    case 'b': return BISHOP;
    case 'r': return ROOK;
    case 'q': return QUEEN;
    case 'k': return KING;
    default: std::cout << "makePieceType(char " << c << ") is illegal char" << std::endl; exit(1);
    }
}

Color makeColor(char c) {
    char cLower = tolower(c);
    if (cLower == c)
	return BLACK;
    else 
	return WHITE;
}

} // end namespace Pieces
#endif
