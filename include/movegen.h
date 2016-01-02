#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <string>
#include <iostream>

#include "types.h"
#include "move.h"
#include "position.h"

class MoveGenerator {
  protected:
  std::vector<Move> generatedMoves;
  Position* pos;

  public:
  MoveGenerator();
  MoveGenerator(Position& position);
  std::vector<Move> getGeneratedMoves();
  void generateMoves();
  void encodeAndAddMove(Square s1, Square s2);
  Move getRandomMove();
};

inline std::vector<Move> MoveGenerator::getGeneratedMoves() {
  return generatedMoves;
}

#endif
