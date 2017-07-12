#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <string>
#include <iostream>
#include <map>
#include <vector>

#include "types.h"
#include "move.h"
#include "position.h"

const std::map<Piece, std::vector<Square>> directions =
    {
        { W_PAWN, { D_NORTH, D_NORTH + D_NORTH, D_NORTH_EAST, D_NORTH_WEST } },
        { B_PAWN, { D_SOUTH, D_SOUTH + D_SOUTH, D_SOUTH_EAST, D_SOUTH_WEST } },
        { W_KNIGHT, { D_NORTH + D_NORTH + D_EAST,
                      D_NORTH + D_NORTH + D_WEST,
                      D_EAST + D_EAST + D_NORTH,
                      D_EAST + D_EAST + D_SOUTH,
                      D_SOUTH + D_SOUTH + D_EAST,
                      D_SOUTH + D_SOUTH + D_WEST,
                      D_WEST + D_WEST + D_SOUTH,
                      D_WEST + D_WEST + D_NORTH } },
        { B_KNIGHT, { D_NORTH + D_NORTH + D_EAST,
                      D_NORTH + D_NORTH + D_WEST,
                      D_EAST + D_EAST + D_NORTH,
                      D_EAST + D_EAST + D_SOUTH,
                      D_SOUTH + D_SOUTH + D_EAST,
                      D_SOUTH + D_SOUTH + D_WEST,
                      D_WEST + D_WEST + D_SOUTH,
                      D_WEST + D_WEST + D_NORTH } },
        { W_BISHOP, { D_NORTH_EAST, D_NORTH_WEST, D_SOUTH_EAST, D_SOUTH_WEST } },
        { B_BISHOP, { D_NORTH_EAST, D_NORTH_WEST, D_SOUTH_EAST, D_SOUTH_WEST } },
        { W_ROOK,   { D_NORTH, D_EAST, D_SOUTH, D_WEST } },
        { B_ROOK,   { D_NORTH, D_EAST, D_SOUTH, D_WEST } },
        { W_QUEEN,  { D_NORTH, D_EAST, D_SOUTH, D_WEST,
                      D_NORTH_EAST, D_NORTH_WEST, D_SOUTH_EAST, D_SOUTH_WEST } },
        { B_QUEEN,  { D_NORTH, D_EAST, D_SOUTH, D_WEST,
                      D_NORTH_EAST, D_NORTH_WEST, D_SOUTH_EAST, D_SOUTH_WEST } },
        { W_KING,  { D_NORTH, D_EAST, D_SOUTH, D_WEST,
                      D_NORTH_EAST, D_NORTH_WEST, D_SOUTH_EAST, D_SOUTH_WEST } },
        { B_KING,  { D_NORTH, D_EAST, D_SOUTH, D_WEST,
                      D_NORTH_EAST, D_NORTH_WEST, D_SOUTH_EAST, D_SOUTH_WEST } }
    };

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
