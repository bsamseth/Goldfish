use chess::{Board, MoveGen};

use super::{cuts, speculate, PvNode, Searcher};
use crate::{
    board::BoardExt,
    movelist::MoveVec,
    newtypes::{Depth, Ply, Value},
    tt,
};

impl Searcher<'_> {
    /// Quiescence search with alpha-beta pruning.
    pub fn quiescence_search<const PV: PvNode>(
        &mut self,
        board: &Board,
        mut alpha: Value,
        beta: Value,
        ply: Ply,
    ) -> Result<Value, Value> {
        debug_assert!(alpha >= -Value::INFINITE && alpha < beta && beta <= Value::INFINITE);
        debug_assert!(PV || (alpha == beta.decrement()));

        self.logger.update_search(ply);

        self.return_evaluation_if_at_forced_leaf(board, ply)?;
        self.return_if_draw(board, ply)?;

        let (tt_data, tt_writer) = self.transposition_table.probe_mut(
            self.stack_state(ply).zobrist,
            ply,
            self.stack_state(ply).halfmove_clock,
        );

        // At non-PV nodes we check for an early TT cutoff
        if !PV {
            if let Some(ref tt_data) = tt_data {
                if tt_data.depth >= Depth::ZERO {
                    if let Some(bounded_value) = tt_data.bounded(alpha) {
                        return Ok(bounded_value);
                    }
                }
            }
        }

        let (mut best_value, eval) = cuts::lower_bound_eval(board, tt_data.as_ref());
        debug_assert!(!board.in_check() || best_value == -Value::INFINITE);

        // Standing pat. Note: If we are in check, this condition is never true.
        if best_value >= beta {
            if tt_data.is_none() {
                // SAFETY: The tt writer was created in this search frame, so the entry is valid.
                unsafe {
                    tt_writer.save::<PV>(&tt::EntryWriterOpts {
                        value: Some(best_value),
                        bound: tt::Bound::Lower,
                        depth: Depth::UNSEARCHED,
                        mv: None,
                        eval,
                        ply,
                    });
                }
            }
            return Ok(best_value);
        }
        alpha = alpha.max(best_value);

        cuts::full_delta_pruning(board, best_value, alpha)?;

        let mut moves = MoveGen::new_legal(board);
        if board.in_check() && moves.len() == 0 {
            return Ok(Value::mated_in(ply));
        } else if !board.in_check() {
            // If we're not in check, we only search captures.
            // In check we should also consider evasions.
            let targets = board.color_combined(!board.side_to_move());
            moves.set_iterator_mask(*targets);
        }
        let moves = MoveVec::from(moves)
            .mvv_lva_rated(board)
            .with_history_stats(&self.history_stats)
            .sorted();

        let mut new_board = *board;
        let mut best_move = None;
        for mv in moves.iter().map(|entry| entry.mv) {
            if speculate::delta_pruning(board, mv, best_value, alpha) {
                continue;
            }
            self.make_move(board, mv, &mut new_board, ply);

            let value = -Value::from(self.quiescence_search::<PV>(
                &new_board,
                -beta,
                -alpha,
                ply.increment(),
            ));

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

        if board.in_check() && moves.is_empty() {
            return Ok(Value::mated_in(ply));
        }

        // SAFETY: The tt writer was created in this search frame, so the entry is valid.
        unsafe {
            tt_writer.save::<PV>(&tt::EntryWriterOpts {
                value: Some(best_value),
                bound: if best_value >= beta {
                    tt::Bound::Lower
                } else {
                    tt::Bound::Upper
                },
                depth: Depth::ZERO,
                mv: best_move,
                eval,
                ply,
            });
        }

        Ok(best_value)
    }
}
