#include <iostream>
#include <string>

#include "move.h"
#include "position.h"
#include "bitboards.h"


using std::cout;
using std::endl;
using std::string;
using Bitboards::prettyString;
int main() {
    Position pos;
    pos.setFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Move move (SQ_E7, SQ_E5), move2 (SQ_G1, SQ_F3), move3 (SQ_A7, SQ_A5), move4 (SQ_B1, SQ_A3);
    pos.doMove(move);
    pos.doMove(move2);
    pos.doMove(move3);
    pos.doMove(move4);
    cout << Bitboards::prettyString(pos.getBoardForColor(WHITE) | pos.getBoardForColor(BLACK)) << endl;
    for (int r = RANK_8; r >= RANK_1; --r) {
    	for (int f = FILE_A; f <= FILE_H; ++f) {
    	    cout << "(" << r << "," << f << "): " << pos.board[8*r+f] << " | ";
    	}
	cout << "\n";
    }
    cout << makePieceType(pos.board[SQ_B1]) << endl;
    cout << pos.halfmoveClock << endl;
    return 0;
} 
