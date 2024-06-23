use chess::{Board, MoveGen};

use super::cuts;
use super::PvNode;
use super::Searcher;
use crate::board::BoardExt;
use crate::evaluate::Evaluate;
use crate::movelist::MoveVec;
use crate::newtypes::{Depth, Ply, Value};
use crate::tt::Bound;

impl Searcher<'_> {
    /// Negamax search with alpha-beta pruning.
    pub fn negamax<const PV: PvNode>(
        &mut self,
        board: &Board,
        mut depth: Depth,
        mut alpha: Value,
        mut beta: Value,
        ply: Ply,
    ) -> Result<Value, Value> {
        // Step 1: Early return if we don't have to expand this node:
        self.return_evaluation_if_at_forced_leaf(board, ply)?;
        self.return_if_draw(board, ply)?;

        cuts::mate_distance_pruning(&mut alpha, &mut beta, ply)?;
        let tt_move = self.get_bounds_and_move_from_tt(&mut alpha, &mut beta, ply, depth)?;
        self.check_tablebase(board, &mut alpha, &mut beta, ply, depth)?;

        // Step 2: Quiescence search if at max depth.
        if depth == Depth::new(0) {
            return self.quiescence_search::<PV>(board, alpha, beta, ply);
        }

        // Step 3: Expand the node.
        self.logger.update_search(ply); // Only now do we say we're at a new node.

        // Step 4: Speculative extensions / pruning.
        if board.in_check() {
            depth += Depth::new(1); // Extend search if in check.
        } else {
            self.stack_state_mut(ply).eval = board.evaluate();
            self.futility_pruning::<PV>(board, alpha, beta, &mut depth, ply)?;
            self.null_move_pruning(board, &mut beta, depth, ply)?;
        }

        // Step 5: Internal iterative deepening.
        let tt_move =
            self.internal_iterative_deepening::<PV>(board, tt_move, depth, alpha, beta, ply);

        // Step 6: Move generation and ordering.
        let moves = MoveVec::from(MoveGen::new_legal(board))
            .mvv_lva_rated(board)
            .with_history_stats(&self.history_stats)
            .sort_with_preference(
                [
                    tt_move,
                    self.stack_state(ply).killers[0],
                    self.stack_state(ply).killers[1],
                ]
                .iter()
                .filter_map(|x| *x),
            );

        // Step 7: Recursively search all possible moves.
        let mut best_value = -Value::INFINITE;
        let mut best_move = None;
        let mut new_board = *board;
        for (mv_nr, mv) in moves.iter().map(|entry| entry.mv).enumerate() {
            self.make_move(board, mv, &mut new_board, ply);

            let value = self.pv_search::<PV>(&new_board, depth, alpha, beta, ply, mv_nr);

            if self.should_stop() {
                // If we're stopping, we don't trust the value, because it was likely cut off.
                // Rely on whatever we've found so far.
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

        // Step 8: Store and return result.
        let bound = if moves.len() == 0 {
            best_value = if board.in_check() {
                Value::mated_in(ply)
            } else {
                Value::DRAW
            };
            Bound::Exact
        } else if best_value >= beta {
            Bound::Lower
        } else if PV && best_move.is_some() {
            Bound::Exact
        } else {
            Bound::Upper
        };

        self.stack_state_mut(ply).update_killer(best_move);
        self.update_history_stats(best_move);
        self.transposition_table.store(
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
