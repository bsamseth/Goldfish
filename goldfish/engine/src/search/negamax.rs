use chess::{Board, MoveGen};

use super::Searcher;
use crate::board::BoardExt;
use crate::movelist::MoveVec;
use crate::newtypes::{Depth, Ply, Value};
use crate::tt::Bound;

impl Searcher {
    /// Negamax search with alpha-beta pruning.
    pub fn negamax(
        &mut self,
        board: &Board,
        depth: Depth,
        mut alpha: Value,
        mut beta: Value,
        ply: Ply,
    ) -> Result<Value, Value> {
        self.return_evaluation_if_at_forced_leaf(board, ply)?;
        self.return_if_draw(board, ply)?;

        let alpha_orig = alpha; // Save original alpha for later determinaiton of search bound type.
        let tt_move = self.get_bounds_and_move_from_tt(&mut alpha, &mut beta, ply, depth)?;

        if depth == Depth::new(0) {
            return self.quiescence_search(board, alpha, beta, ply);
        }

        self.logger.update_search(ply);

        Searcher::mate_distance_pruning(&mut alpha, &mut beta, ply)?;

        let moves = MoveVec::from(MoveGen::new_legal(board))
            .mvv_lva_rated(board)
            .sort_with_preference(
                [
                    tt_move,
                    self.stack_state(ply).killers[0],
                    self.stack_state(ply).killers[1],
                ]
                .iter()
                .filter_map(|x| *x),
            );

        let mut best_value = -Value::INFINITE;
        let mut best_move = None;
        let mut new_board = *board;

        for (mv_nr, mv) in moves.iter().map(|entry| entry.mv).enumerate() {
            self.make_move(board, mv, &mut new_board, ply);

            let value = self.pv_search(&new_board, depth, alpha, beta, ply, mv_nr);

            if self.should_stop() {
                // If we're stopping, we don't trust the value, because it was likely cut off.
                // Rely on whatever we've found so far on only.
                return Err(best_value);
            }

            if value > best_value {
                best_value = value;
                best_move = Some(mv);

                if value > alpha {
                    alpha = value;
                    if value >= beta {
                        break;
                    }
                }
            }
        }

        let bound = if moves.len() == 0 {
            best_value = if board.in_check() {
                Value::mated_in(ply)
            } else {
                Value::DRAW
            };
            Bound::Exact
        } else if best_value <= alpha_orig {
            Bound::Upper
        } else if best_value >= beta {
            Bound::Lower
        } else {
            Bound::Exact
        };

        self.stack_state_mut(ply).update_killer(best_move);
        self.transposition_table.write().unwrap().store(
            self.stack_state(ply).zobrist,
            best_move,
            bound,
            best_value,
            depth,
            ply,
        );

        Ok(best_value)
    }
}
