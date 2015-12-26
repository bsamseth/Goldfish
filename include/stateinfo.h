#ifndef STATEINFO_H
#define STATEINFO_H

#include "types.h"


struct StateInfo {
  static int ID;
  int id;
  StateInfo();
  StateInfo(bool root);
  void reset();
  Piece lastMove_originPiece;
  Piece lastMove_destinationPiece;
  int previous_halfmoveClock;
  int previous_fullmoveNumber;
  Square lastMove_enpassantTarget;
  StateInfo* previous;
};



#endif
