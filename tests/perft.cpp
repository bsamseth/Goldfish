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
  for (auto move : generator.getGeneratedMoves()) {
    pos.doMove(move);
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

TEST (Perft, perft3) {
  Position pos = Position(STARTING_FEN);
  int perft_res = Perft(pos, 3);
  ASSERT_EQ(8902, perft_res);
}

TEST (Perft, extra) {
  Position pos = Position("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  int perft_res = Perft(pos, 1);
  ASSERT_EQ(48, perft_res);
}
