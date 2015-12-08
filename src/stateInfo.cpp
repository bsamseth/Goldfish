#include "stateinfo.h"
#include <iostream>
int StateInfo::ID = 0;

StateInfo::StateInfo() {
  id = ID++;
  lastMove_originPiece = NO_PIECE;
  lastMove_destinationPiece = NO_PIECE;
  previous_halfmoveClock = 0;
  previous_fullmoveNumber = 0;
  lastMove_enpassantTarget = NO_SQUARE;
  previous = nullptr;
}

void StateInfo::reset() {
  lastMove_originPiece = NO_PIECE;
  lastMove_destinationPiece = NO_PIECE;
  previous_halfmoveClock = 0;
  previous_fullmoveNumber = 0;
  lastMove_enpassantTarget = NO_SQUARE;
  previous = nullptr;
}
