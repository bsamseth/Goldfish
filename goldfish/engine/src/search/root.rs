use super::Searcher;
use crate::movelist::MoveEntry;
use crate::newtypes::{Depth, Ply, Value};

impl Searcher {
    /// Search the root position to `depth` moves deep.
    pub fn search_root(&mut self, depth: Depth, mut alpha: Value, beta: Value) {
        assert!(depth > Depth::new(0), "depth must be greater than 0");

        // Reset all values, so the best move as determined at this new (larger) depth is pushed to the front
        self.root_moves.iter_mut().for_each(|m| {
            *m = MoveEntry::new(m.mv);
        });

        let root_board = self.root_position;
        let mut board = root_board;

        for (mv_nr, mv) in self.root_moves.clone().iter().enumerate() {
            self.logger.set_current_move(mv.mv, mv_nr + 1);
            self.logger.send_status();

            self.make_move(&root_board, mv.mv, &mut board, Ply::new(0));

            let value = self.pv_search(&board, depth, alpha, beta, Ply::new(0), mv_nr);

            if self.should_stop() {
                // If we're stopping, we don't trust the value, because it was likely cut off.
                // Rely on whatever we've found so far on only.
                return;
            }

            if value > alpha {
                alpha = value;

                self.root_moves[mv_nr].value = value;

                if value >= beta {
                    return;
                }

                self.logger.send_move(
                    &self.root_moves[mv_nr],
                    &self.build_pv(depth),
                    self.transposition_table.read().unwrap().hashfull(),
                );
            }
        }
    }
}
