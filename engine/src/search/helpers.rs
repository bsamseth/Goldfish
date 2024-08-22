use chess::{Board, ChessMove};

use super::stackstate::StackState;
use super::Searcher;
use crate::board::BoardExt;
use crate::newtypes::{Depth, Ply, Value};
use crate::tt;

impl Searcher<'_> {
    /// Return `true` if the search should stop.
    ///
    /// The search should stop if any of the following conditions are met:
    ///
    ///   - The stop signal has been set.
    ///   - The time limit has been reached.
    ///   - The node limit has been reached.
    pub fn should_stop(&self) -> bool {
        self.stop_signal.load(std::sync::atomic::Ordering::Relaxed)
            || self.logger.exceeds_time_limit(self.limits.movetime)
            || self.logger.exceeds_node_limit(self.limits.nodes)
    }

    /// Return the principal variation for the current position.
    ///
    /// The principal variation is the sequence of moves that the engine thinks is the best.
    /// It is used to display the engine's thinking process.
    ///
    /// We use the transposition table to build the PV on demand. This is not the most efficient
    /// way to build the PV itself, but it removes complexity (and cycles) from the main search
    /// code path. Only when a new best move is found at the root (for a given depth) do we need
    /// to build the PV, which means this doesn't impact the search speed.
    ///
    /// Another potential downside is that the PV might be cut short if a replacement occurs in the
    /// table. For a reasonable table size this should be rare, so this approach is the less
    /// intrusive one.
    ///
    /// The PV will contain at most `depth` moves, as any more than this would likely be speculative.
    pub fn build_pv(&self, depth: Depth) -> Vec<ChessMove> {
        let mut pv = Vec::new();
        let mut board = self.root_position;

        // The root position is _not_ in the table, so we manually pick out the best root move.
        // The root move list is only sorted by the previous depth's values, so we must determine
        // it on demand.
        let first_move = self.root_moves.iter().max_by_key(|r| r.value).unwrap().mv;
        board = board.make_move_new(first_move);
        pv.push(first_move);

        while let Some(tt::Data { mv: Some(mv), .. }) =
            self.transposition_table
                .probe(board.get_hash(), Ply::ZERO, 0)
        {
            pv.push(mv);
            board = board.make_move_new(mv);

            if pv.len() >= depth.as_usize() {
                break;
            }
        }

        pv
    }

    /// Make a move on a pre-allocated board, and update the stack state.
    ///
    /// The move must be valid for `board`, and `result` will be the result of
    /// applying `mv` to `board`.
    ///
    /// `ply` should be the current ply count in the search, and _not_ the count after the move
    /// is made. I.e. this should be true: `self.stack_state(ply).zobrist == board.get_hash()`.
    pub fn make_move(&mut self, board: &Board, mv: ChessMove, result: &mut Board, ply: Ply) {
        let current_halfmove = self.stack_state(ply).halfmove_clock;
        let new_ss = self.stack_state_mut(ply.increment());

        if board.halfmove_reset(mv) {
            new_ss.halfmove_clock = 0;
        } else {
            new_ss.halfmove_clock = current_halfmove + 1;
        }
        board.make_move(mv, result);
        new_ss.zobrist = result.get_hash();
    }

    /// Make a null move, return the new board and update the stack states.
    ///
    /// # Panics
    /// Panics if the position is in check, as a null move is not possible in that case.
    pub fn make_null_move(&mut self, board: &Board, ply: Ply) -> Board {
        let board = board
            .null_move()
            .expect("not in check, so null move should be possible");

        // These assignments look a little unsorted, but it was needed for the borrow checker.
        let current_ss = self.stack_state_mut(ply);
        current_ss.null_move = true;
        let new_hmc = current_ss.halfmove_clock + 1;
        let new_ss = self.stack_state_mut(ply.increment());
        new_ss.halfmove_clock = new_hmc;
        new_ss.zobrist = board.get_hash();

        board
    }

    /// Undo a null move.
    ///
    /// This does _not_ undo what [`Searcher::make_null_move`] does to the `ply+1` stack state.
    pub fn undo_null_move(&mut self, ply: Ply) {
        self.stack_state_mut(ply).null_move = false;
    }

    /// Return `true` if the current position is a draw.
    pub fn is_draw(&self, board: &Board, ply: Ply) -> bool {
        if self.stack_state(ply).halfmove_clock >= 100 || board.has_insufficient_material() {
            return true;
        }

        // Check for repetition.
        // Check positions from the last halfmove clock reset, and return true if we've seen the
        // same position twice before. Positions are treated as equal by their Zobrist keys.
        let ply = (self.root_position_ply + ply).as_usize();
        let position_count = self.ss[ply.saturating_sub(self.ss[ply].halfmove_clock)..ply]
            .iter()
            .filter({
                let latest_zobrist = self.ss[ply].zobrist;
                move |s| s.zobrist == latest_zobrist
            })
            .count();
        position_count >= 2
    }

    /// Return a reference to the stack state for the given ply.
    pub fn stack_state(&self, ply: Ply) -> &StackState {
        &self.ss[(self.root_position_ply + ply).as_usize()]
    }
    /// Return a mutable reference to the stack state for the given ply.
    pub fn stack_state_mut(&mut self, ply: Ply) -> &mut StackState {
        &mut self.ss[(self.root_position_ply + ply).as_usize()]
    }

    /// Remove moves from the root move list that don't preserve the WDL value (if available).
    ///
    /// A tablebase must be initialized, and the root position must be in the tablebase.
    pub fn filter_root_moves_using_tb(&mut self) {
        let hmc = self.stack_state(Ply::ZERO).halfmove_clock;

        if let Some((wdl, filter)) = self
            .tablebase
            .and_then(|tb| tb.probe_dtz(&self.root_position, hmc))
        {
            self.logger.tb_hit();
            self.root_moves.retain(|r| filter(&r.mv));

            if self.root_moves.len() < 2 {
                assert!(self.root_moves.len() == 1);
                self.root_moves[0].value = wdl.into();
                self.logger
                    .send_move(&self.root_moves[0], &[self.root_moves[0].mv], 0);
            }
        }
    }

    pub fn update_history_stats(&mut self, mv: Option<ChessMove>) {
        if let Some(mv) = mv {
            self.history_stats[mv.get_source().to_index()][mv.get_dest().to_index()] += 1;
        }
    }
}

impl From<Result<Value, Value>> for Value {
    /// Collapse a `Result<Value, Value>` into a `Value`.
    fn from(result: Result<Value, Value>) -> Self {
        match result {
            Ok(value) | Err(value) => value,
        }
    }
}

#[cfg(test)]
mod tests {
    use std::sync::Arc;

    use uci::Position;

    use crate::tt::TranspositionTable;

    use super::*;

    #[test]
    fn test_is_draw_by_50_move_rule() {
        let board = Board::default();
        let position = Position {
            start_pos: board,
            moves: vec![],
            starting_halfmove_clock: 0,
        };
        let mut tt = TranspositionTable::default();
        let mut searcher = Searcher::new(&position, &[], Arc::default(), &mut tt, None);

        searcher.stack_state_mut(Ply::ZERO).halfmove_clock = 100;
        searcher.stack_state_mut(Ply::ONE).halfmove_clock = 99;
        assert!(searcher.is_draw(&board, Ply::ZERO));
        assert!(!searcher.is_draw(&board, Ply::ONE));
    }

    #[test]
    fn test_is_draw_by_insufficient_material() {
        for fen in [
            "k7/8/8/8/8/8/8/KB6 w - - 0 1",
            "8/8/nk6/8/8/8/8/K7 b - - 0 1",
        ] {
            let board: Board = fen.parse().unwrap();
            assert!(board.has_insufficient_material());
            let position = Position {
                start_pos: board,
                moves: vec![],
                starting_halfmove_clock: 0,
            };
            let mut tt = TranspositionTable::default();
            let searcher = Searcher::new(&position, &[], Arc::default(), &mut tt, None);
            assert!(searcher.is_draw(&board, Ply::ZERO));
        }
    }

    #[test]
    fn test_is_draw_by_repetition() {
        let position: Position = "fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves g1f3 b8c6 f3g1 c6b8 g1f3 b8c6 f3g1".parse().unwrap();
        let mut tt = TranspositionTable::default();
        let mut searcher = Searcher::new(&position, &[], Arc::default(), &mut tt, None);
        assert!(!searcher.is_draw(&position.start_pos, Ply::ZERO));

        let board = searcher.root_position;
        let mut new_board = board;
        searcher.make_move(&board, "c6b8".parse().unwrap(), &mut new_board, Ply::ZERO);

        assert!(searcher.is_draw(&board, Ply::ONE));
    }
}
