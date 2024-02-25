use std::sync::{Arc, RwLock};

use chess::{Board, ChessMove, MoveGen};
use uci::UciPosition;

use super::stackstate::StackState;
use super::Searcher;
use crate::board::BoardExt;
use crate::limits::Limits;
use crate::logger::Logger;
use crate::movelist::MoveVec;
use crate::newtypes::{Depth, Ply, Value};
use crate::stop_signal::StopSignal;
use crate::tablebase::Tablebase;
use crate::tt::{Bound, TranspositionTable};

impl Searcher {
    /// Create a new [`Searcher`].
    pub fn new(
        position: UciPosition,
        options: &[uci::GoOption],
        stop_signal: StopSignal,
        transposition_table: Arc<RwLock<TranspositionTable>>,
        tablebase: Option<Arc<Tablebase>>,
    ) -> Self {
        let mut board = position.start_pos;
        let mut root_position = board;
        let mut halfmove_clock = position.starting_halfmove_clock;
        for mv in position.moves {
            if board.halfmove_reset(mv) {
                halfmove_clock = 0;
            } else {
                halfmove_clock += 1;
            }
            board.make_move(mv, &mut root_position);
            board = root_position;
        }

        let mut stack_states = [StackState::default(); Ply::MAX.as_usize() + 1];
        stack_states[0] = StackState {
            halfmove_clock,
            zobrist: root_position.get_hash(),
            killers: [None; 2],
        };

        let mut root_moves: MoveVec = MoveGen::new_legal(&root_position).into();

        if let Some(probe) = tablebase
            .as_ref()
            .and_then(|tb| tb.probe_root(&root_position, halfmove_clock))
        {
            probe.filter_moves(&mut root_moves);
        }

        Self {
            root_position,
            ss: stack_states,
            limits: Limits::from(options),
            logger: Logger::new(),
            stop_signal,
            root_moves: root_moves.mvv_lva_rated(&root_position).sorted(),
            transposition_table,
            tablebase,
        }
    }

    /// Return `true` if the search should stop.
    ///
    /// The search should stop if any of the following conditions are met:
    ///
    ///   - The stop signal has been set.
    ///   - The time limit has been reached.
    ///   - The node limit has been reached.
    pub fn should_stop(&self) -> bool {
        self.stop_signal.check()
            || self.logger.search_start_time.elapsed().as_millis() as usize >= self.limits.movetime
            || self.logger.total_nodes >= self.limits.nodes
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

        while let Some((Some(mv), Bound::Exact, _)) = self.transposition_table.read().unwrap().get(
            board.get_hash(),
            Depth::new(0),
            Ply::new(0),
        ) {
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
        let new_ss = self.stack_state_mut(ply + Ply::new(1));

        if board.halfmove_reset(mv) {
            new_ss.halfmove_clock = 0;
        } else {
            new_ss.halfmove_clock = current_halfmove + 1;
        }
        board.make_move(mv, result);
        new_ss.zobrist = result.get_hash();
    }

    /// Return `true` if the current position is a draw.
    pub fn is_draw(&self, board: &Board, ply: Ply) -> bool {
        if self.stack_state(ply).halfmove_clock >= 100 || board.has_insufficient_material() {
            return true;
        }

        // Check for repetition.
        // Check positions from the last halfmove clock reset, and return true if we've seen the
        // same position twice before. Positions are treated as equal by their Zobrist keys.
        let ply = ply.as_usize();
        self.ss[ply.saturating_sub(self.ss[ply].halfmove_clock)..ply]
            .iter()
            .filter({
                let latest_zobrist = self.ss[ply].zobrist;
                move |s| s.zobrist == latest_zobrist
            })
            .count()
            >= 2
    }

    /// Return a reference to the stack state for the given ply.
    pub fn stack_state(&self, ply: Ply) -> &StackState {
        &self.ss[ply.as_usize()]
    }
    /// Return a mutable reference to the stack state for the given ply.
    pub fn stack_state_mut(&mut self, ply: Ply) -> &mut StackState {
        &mut self.ss[ply.as_usize()]
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
