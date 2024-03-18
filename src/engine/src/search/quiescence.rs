use chess::{Board, MoveGen};

use super::{cuts, speculate, Searcher};
use crate::board::BoardExt;
use crate::movelist::MoveVec;
use crate::newtypes::{Ply, Value};

impl Searcher {
    /// Quiescence search with alpha-beta pruning.
    pub fn quiescence_search(
        &mut self,
        board: &Board,
        mut alpha: Value,
        beta: Value,
        ply: Ply,
    ) -> Result<Value, Value> {
        self.logger.update_search(ply);

        self.return_evaluation_if_at_forced_leaf(board, ply)?;
        self.return_if_draw(board, ply)?;

        let mut best_value = cuts::standing_pat(board, &mut alpha, beta)?;
        cuts::full_delta_pruning(board, best_value, alpha)?;

        let mut moves = MoveGen::new_legal(board);
        cuts::return_if_no_moves(&moves, board, ply)?;

        // If we're not in check, we only search captures.
        // In check we should also consider evasions.
        if !board.in_check() {
            let targets = board.color_combined(!board.side_to_move());
            moves.set_iterator_mask(*targets);
        }
        let moves = MoveVec::from(moves).mvv_lva_rated(board).sorted();

        let mut new_board = *board;
        for mv in moves.iter().map(|entry| entry.mv) {
            if speculate::delta_pruning(board, mv, best_value, alpha) {
                continue;
            }
            self.make_move(board, mv, &mut new_board, ply);

            let value =
                -Value::from(self.quiescence_search(&new_board, -beta, -alpha, ply + Ply::new(1)));

            if self.should_stop() {
                // If we're stopping, we don't trust the value, because it was likely cut off.
                // Rely on whatever we've found so far on only.
                return Err(best_value);
            }

            best_value = best_value.max(value);

            if value > alpha {
                alpha = value;

                if value >= beta {
                    break;
                }
            }
        }

        Ok(best_value)
    }
}
