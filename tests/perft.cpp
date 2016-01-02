#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include "types.h"
#include "position.h"
#include "move.h"
#include "movegen.h"

using namespace std;


int Perft(Position& pos, int depth) {
  int nodes = 0;
 
  if (depth == 0) return 1;

  MoveGenerator generator = MoveGenerator(pos);
  generator.generateMoves();
  auto all_moves = generator.getGeneratedMoves();
  unsigned n_moves = (unsigned) all_moves.size();

  for (unsigned i = 0; i < n_moves; i++) {
    pos.doMove(all_moves[i]);
    nodes += Perft(pos, depth - 1);
    pos.undoMove();
  }
  return nodes;
}


TEST (Perft, perft1) {
  Position pos = Position(STARTING_FEN);
  int perft_res = Perft(pos, 1);
  ASSERT_EQ(20, perft_res);
}

TEST (Perft, perft2) {
  Position pos = Position(STARTING_FEN);
  int perft_res = Perft(pos, 2);
  ASSERT_EQ(400, perft_res);
}

TEST (Perft, DISABLED_perft3) {
  Position pos = Position(STARTING_FEN);
  int perft_res = Perft(pos, 3);
  ASSERT_EQ(8902, perft_res);
}


