#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>


#include "types.h"
#include "move.h"
#include "position.h"
#include "bitboards.h"
#include "movegen.h"


using std::cout;
using std::endl;
using std::string;
using std::vector;
using Bitboards::prettyString;

void handler(int sig);

int main() {
  signal(SIGSEGV, handler); // do something on seg fault other than crashing
  
  Position pos;
  pos.setFromFEN(STARTING_FEN);
  pos.setFromFEN("rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/R1BQKBNR b KQkq - 0 1");

  Move move (SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_MOVE), move2 (SQ_F7, SQ_F6), move3 (SQ_E4, SQ_E5), move4 (SQ_F6, SQ_E5, CAPTURE_MOVE);
  Move pawnPush(SQ_A2, SQ_A3), pawnDoublePush(SQ_B2, SQ_B3, DOUBLE_PAWN_PUSH_MOVE);
  Move pawnDoublePushC(SQ_C2, SQ_C4, DOUBLE_PAWN_PUSH_MOVE), pawnDoublePushD(SQ_D2, SQ_D4, DOUBLE_PAWN_PUSH_MOVE);
  Move bugPawn(SQ_H2, SQ_A4), enpas(SQ_D4, SQ_E3);

  cout << pos.str() << endl;

  
  MoveGenerator gen = MoveGenerator(pos);
  gen.generateMoves();

  auto all_moves = gen.getGeneratedMoves();
  for (int i = 0; i < (int) all_moves.size(); i++) {
    cout << all_moves[i].str() << endl;
  }
  cout << "Found " << all_moves.size() << " moves." << endl;

  
  return 0;
} 

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}
