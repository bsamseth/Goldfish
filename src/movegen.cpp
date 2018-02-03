#include <iostream>
#include <cstdlib>
#include <string>
#include <map>
#include <vector>

#include "types.h"
#include "bitboards.h"
#include "movegen.h"
#include "move.h"
#include "position.h"


using std::string;
using std::cout;
using std::endl;


MoveGenerator::MoveGenerator() {}

/*
 * Initialize the generator with a position.
 */
MoveGenerator::MoveGenerator(Position& position) {
    pos = &position;
}

void MoveGenerator::generateMoves() {
    Square cSquare, target;
    for (int rank = RANK_1; rank <= RANK_8; rank++) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            cSquare = Square(rank*8 + file);
            if (!pos->occupied(cSquare, pos->sideToMove)) {
                continue;
            }

            Piece p = pos->board[cSquare];
            PieceType pt = makePieceType(p);
            for (auto d : directions.find(p)->second) {
                target = cSquare + d;
                while (true) {
                    if ( target < SQ_A1 or target > SQ_H8 ) break;
                    if (pos->occupied(target, pos->sideToMove)) break;
                    Piece q = pos->board[target];

                    /* PAWN RULES */
                    // Pawn push with blocking piece.
                    if (pt == PAWN and (d == D_NORTH or d == D_NORTH + D_NORTH
                                or d == D_SOUTH or d == D_SOUTH + D_SOUTH)
                            and q != NO_PIECE)
                        break;
                    // Double pawn push not on starting rank or blocking piece.
                    if (pt == PAWN and d == D_NORTH + D_NORTH
                            and (((1ULL << cSquare) & RANK_2_BB) == 0 or pos->occupied(cSquare + D_NORTH)))
                        break;
                    if (pt == PAWN and d == D_SOUTH + D_SOUTH
                            and (((1ULL << cSquare) & RANK_7_BB) == 0 or pos->occupied(cSquare + D_SOUTH)))
                        break;
                    // Pawn capture with no piece to capture (and not ep).
                    if (pt == PAWN and (d == D_NORTH_EAST or d == D_NORTH_WEST or
                                d == D_SOUTH_EAST or d == D_SOUTH_WEST)
                            and q == NO_PIECE and target != pos->enpassantTarget)
                        break;

                    /* PIECE RULES */
                    int d_rank = abs(rank_diff(cSquare, target));
                    int d_file = abs(file_diff(cSquare, target));

                    if (pt == PAWN and d_file > 1) break;
                    if (pt == KNIGHT and (d_rank + d_file != 3)) break;
                    if (pt == BISHOP and d_file != d_rank) break;
                    if (pt == ROOK and not ((d_file == 0) != (d_rank == 0))) break;
                    if ((pt == QUEEN or pt == KING) and not ((d_file == d_rank) or ((d_file == 0) != (d_rank == 0)))) break;

                    encodeAndAddMove(cSquare, target);

                    // Stop crawlers from sliding, and sliding after captures.
                    if (pt == PAWN or pt == KNIGHT or pt == KING or q != NO_PIECE)
                        break;

                    // Castle by sliding the rock next to the king.
                    if (((cSquare == SQ_A1 and (pos->castlingRights & WHITE_OOO)) or
                                (cSquare == SQ_A8 and (pos->castlingRights & BLACK_OOO)))
                            and makePieceType(pos->board[target + D_EAST]) == KING
                            and makeColor(pos->board[target + D_EAST]) == pos->sideToMove)
                        encodeAndAddMove(target + D_EAST, target + D_WEST);
                    if (((cSquare == SQ_H1 and (pos->castlingRights & WHITE_OO)) or
                                (cSquare == SQ_H8 and (pos->castlingRights & BLACK_OO)))
                            and makePieceType(pos->board[target + D_WEST]) == KING
                            and makeColor(pos->board[target + D_WEST]) == pos->sideToMove)
                        encodeAndAddMove(target + D_WEST, target + D_EAST);

                    target += d;
                }
            }
        }
    }
}

/*
 * Encode the suggested move, and then add it to the generatedMoves vector.
 * It assumes that pos->legal(s1, s2) has returned true.
 */
void MoveGenerator::encodeAndAddMove(Square s1, Square s2) {
    Piece p1 = pos->board[s1], p2 = pos->board[s2];
    Square up = makeColor(p1) == WHITE ? D_NORTH : D_SOUTH;
    Square from = s1;
    Square to   = s2;

    // castling
    if (makePieceType(p1) == KING) {
        if (s1 == SQ_E1 || s1 == SQ_E8) {
            if (s2 == SQ_G1 || s2 == SQ_G8) {
                generatedMoves.push_back(Move(from, to, KING_CASTLE_MOVE));
                return;
            } else if (s2 == SQ_C1 || s2 == SQ_C8) {
                generatedMoves.push_back(Move(from, to, QUEEN_CASTLE_MOVE));
                return;
            }
        }
    }

    // en passant?
    if (pos->enpassantTarget != NO_SQUARE && makePieceType(p1) == PAWN && s2 == pos->enpassantTarget) {
        generatedMoves.push_back(Move(from, to, ENPASSANT_CAPTURE_MOVE));
        return;
    }

    // promotion?
    if (makePieceType(p1) == PAWN && (s2 & RANK_8_BB || s2 & RANK_8_BB) )  {
        // generate all the possible promotions
        if (p2 != NO_PIECE) { // promo capture?
            generatedMoves.push_back(Move(from, to, KNIGHT_PROMO_CAPTURE_MOVE));
            generatedMoves.push_back(Move(from, to, BISHOP_PROMO_CAPTURE_MOVE));
            generatedMoves.push_back(Move(from, to, ROOK_PROMO_CAPTURE_MOVE));
            generatedMoves.push_back(Move(from, to, QUEEN_PROMO_CAPTURE_MOVE));
        } else {
            generatedMoves.push_back(Move(from, to, KNIGHT_PROMO_MOVE));
            generatedMoves.push_back(Move(from, to, BISHOP_PROMO_MOVE));
            generatedMoves.push_back(Move(from, to, ROOK_PROMO_MOVE));
            generatedMoves.push_back(Move(from, to, QUEEN_PROMO_MOVE));
        }
        return;
    }

    // capture?
    if (p2 != NO_PIECE) {
        generatedMoves.push_back(Move(from, to, CAPTURE_MOVE));
        return;
    }


    // double pawn push?
    if (makePieceType(p1) == PAWN && (int(to) - int(from) == 2*int(up))) {
        generatedMoves.push_back(Move(from, to, DOUBLE_PAWN_PUSH_MOVE));
        return;
    }

    generatedMoves.push_back(Move(from, to, QUIET_MOVE));
}

Move MoveGenerator::getRandomMove() const {
    int randIndex = rand() % generatedMoves.size();
    return generatedMoves[randIndex];
}



/*
 * Sort moves in decreasing order of static evaluation change,
 * positive for the side to move. Uses an insertion sort where
 * pos->value(move) is only called once per move.
 */
void MoveGenerator::sortMoves() {
    std::vector<Move> sorted = generatedMoves;
    int values[generatedMoves.size()];
    for (int i = 0; i < generatedMoves.size(); ++i) {
        Move m = generatedMoves[i];
        int m_val = pos->value(m);
        int j = i;
        while (j > 0 and values[j-1] < m_val) {
            // Swap i and j.
            values[j] = values[j-1];
            sorted[j] = sorted[j-1];
            --j;
        }
        values[j] = m_val;
        sorted[j] = m;
    }
    generatedMoves = sorted;
}

void MoveGenerator::addKiller(const Move &killer) {
    for (auto it = generatedMoves.begin(); it != generatedMoves.end(); ++it) {
        if (killer == *it) {
            auto x = std::move(*it);
            generatedMoves.erase(it);
            generatedMoves.insert(generatedMoves.begin(), std::move(x));
            break;
        }
    }
}

