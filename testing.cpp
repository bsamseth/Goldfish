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
    Move move (SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_MOVE), move2 (SQ_F7, SQ_F6), move3 (SQ_E4, SQ_E5), move4 (SQ_F6, SQ_E5, CAPTURE_MOVE);
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
    // cout << makePieceType(pos.board[SQ_B1]) << endl;
    // cout << pos.halfmoveClock << endl;
    cout << "Move from: " << move.getFrom() << endl;
    cout << "Move to: " <<  move.getTo() << endl;
    cout << "Move flag: " << move.getFlag() << endl;
    cout << (move.capture() ? (move.promotion() ? "Promotion capture" : "Capture")
	     : (move.promotion() ? "Promotion" : "Not capture nor promotion")) << endl;
    cout << "move = " << move.getInteger() << endl;
    return 0;
} 
