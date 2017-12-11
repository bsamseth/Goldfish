#include "stateinfo.h"
#include <iostream>
int StateInfo::ID = 0;

StateInfo::StateInfo() {
  id = ID++;
  lastMove_originPiece = NO_PIECE;
  lastMove_destinationPiece = NO_PIECE;
  previous_halfmoveClock = 0;
  previous_fullmoveNumber = 0;
  previous_castlingRights = NO_CASTLING;
  lastMove_enpassantTarget = NO_SQUARE;
  lastMove_kingpassantTarget = NO_SQUARE;
  previous = nullptr;
}


StateInfo::StateInfo(bool root) {
  id = ID++;
  lastMove_originPiece = NO_PIECE;
  lastMove_destinationPiece = NO_PIECE;
  previous_halfmoveClock = 0;
  previous_fullmoveNumber = 0;
  previous_castlingRights = ANY_CASTLING;
  lastMove_enpassantTarget = NO_SQUARE;
  lastMove_kingpassantTarget = NO_SQUARE;
  previous = this;
}

void StateInfo::reset() {
  lastMove_originPiece = NO_PIECE;
  lastMove_destinationPiece = NO_PIECE;
  previous_halfmoveClock = 0;
  previous_fullmoveNumber = 0;
  previous_castlingRights = ANY_CASTLING;
  lastMove_enpassantTarget = NO_SQUARE;
  lastMove_kingpassantTarget = NO_SQUARE;
  previous = nullptr;
}
