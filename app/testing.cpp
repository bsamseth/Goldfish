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
  pos.setFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  // pos.setFromFEN("rnbqkbnr/pppppppp/8/8/3p4/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  Move move (SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_MOVE), move2 (SQ_F7, SQ_F6), move3 (SQ_E4, SQ_E5), move4 (SQ_F6, SQ_E5, CAPTURE_MOVE);
  Move pawnPush(SQ_A2, SQ_A3), pawnDoublePush(SQ_B2, SQ_B3, DOUBLE_PAWN_PUSH_MOVE);
  Move pawnDoublePushC(SQ_C2, SQ_C4, DOUBLE_PAWN_PUSH_MOVE), pawnDoublePushD(SQ_D2, SQ_D4, DOUBLE_PAWN_PUSH_MOVE);
  Move bugPawn(SQ_H2, SQ_A4), enpas(SQ_D4, SQ_E3);

  cout << pos.str() << endl;
  // cout << "occupied(E2) = " << pos.occupied(SQ_E2) << endl;
  // cout << "doing move = " << move.str() << endl;
  // pos.doMove(move);

  // cout << pos.str() << endl;

  // cout << "occupied(E2) = " << pos.occupied(SQ_E2) << endl;
  
  // cout << "undoing move " << move.str() << endl;
  // pos.undoMove();

  // cout << pos.str() << endl;
  

  
  MoveGenerator gen = MoveGenerator(pos);
  gen.generateMoves();

  // auto allMoves = gen.getGeneratedMoves();
  // cout << "Total number of moves found = " << allMoves.size() << endl;
  // for (int i = 0; i < allMoves.size(); i++) {
  //   cout << allMoves[i].str() << endl;
  // }

    
  // cout << "a3 is psudolegal = " << pos.psudoLegal(pawnPush) << endl;
  // cout << "b4 is psudolegal = " << pos.psudoLegal(pawnDoublePush) << endl;
  // cout << "c4 is psudolegal = " << pos.psudoLegal(pawnDoublePushC) << endl;    
  // cout << "d4 is psudolegal = " << pos.psudoLegal(pawnDoublePushD) << endl;
  // cout << "e4 is psudolegal = " << pos.psudoLegal(move) << endl;
  // cout << "bugPawn is psudolegal = " << pos.psudoLegal(bugPawn) << endl;
  // cout << "Enpassant target = " << Square(pos.enpassantTarget) << endl;

  // cout << Bitboards::prettyString((1 << SQ_C2) & RANK_2_BB) << endl;
    
  // pos.doMove(move);
  // cout << "Enpassant target = " << Square(pos.enpassantTarget) << endl;
  // cout << "Enpassant works = " << pos.psudoLegal(enpas) << endl;
  // // pos.doMove(move2);
  // // pos.doMove(move3);
  // // pos.doMove(move4);
  // // pos.undoMove();
  // cout << Bitboards::prettyString(pos.getBoardForColor(WHITE) | pos.getBoardForColor(BLACK)) << endl;
  // for (int r = RANK_8; r >= RANK_1; --r) {
  //   for (int f = FILE_A; f <= FILE_H; ++f) {
  //     cout << "(" << r << "," << f << "): " << pos.board[8*r+f] << " | ";
  //   }
  //   cout << "\n";
  // }
    
  // // cout << makePieceType(pos.board[SQ_B1]) << endl;
  // // cout << pos.halfmoveClock << endl;
  // cout << "Move from: " << move4.getFrom() << endl;
  // cout << "Move to: " <<  move4.getTo() << endl;
  // cout << "Move flag: " << move4.getFlag() << endl;
  // cout << (move4.capture() ? (move4.promotion() ? "Promotion capture" : "Capture")
  // 	   : (move4.promotion() ? "Promotion" : "Not capture nor promotion")) << endl;
  // cout << "move = " << move4.getInteger() << endl;

  // cout << Bitboards::prettyString(FILE_D_BB) << endl;
  // cout << Bitboards::prettyString(RANK_5_BB) << endl;
  // cout << file_diff(SQ_A1, SQ_H8) << " " << rank_diff(SQ_A1, SQ_H8) << endl;
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
