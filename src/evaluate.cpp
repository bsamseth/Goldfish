
#include <iostream>

#include "evaluate.h"

using namespace std;

int evaluate(const Position & pos) {
    cout << endl;
    int score = 0;
    for (Square sq = SQ_A1; sq <= SQ_H8; ++sq) {
        PieceType pt = makePieceType(pos.board[sq]);
        if (pt == NO_PIECE_TYPE) continue;
        if (makeColor(pos.board[sq]) == WHITE) {
            score += piece_values.find(pt)->second[sq];
        } else {
            score -= piece_values.find(pt)->second[SQ_H8 - sq];
        }
    }
    return score;
}
