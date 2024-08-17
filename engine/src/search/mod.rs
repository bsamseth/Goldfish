mod cuts;
mod helpers;
mod negamax;
mod quiescence;
mod root;
mod run;
mod speculate;
mod stackstate;

use std::sync::{atomic::AtomicBool, Arc};

use chess::{Board, ChessMove, MoveGen};

use super::limits::Limits;
use super::logger::Logger;
use super::movelist::MoveVec;
use super::newtypes::Ply;
use super::tt::TranspositionTable;
use crate::board::BoardExt;
use crate::evaluate::Evaluate;
use fathom::Tablebase;
use stackstate::StackState;
use uci::Position;

#[derive(Debug)]
pub struct Searcher<'a> {
    root_position: Board,
    ss: [StackState; Ply::MAX.as_usize() + 1],
    limits: Limits,
    logger: Logger,
    stop_signal: Arc<AtomicBool>,
    root_moves: MoveVec,
    transposition_table: &'a mut TranspositionTable,
    tablebase: Option<&'a Tablebase>,
    history_stats: [[usize; 64]; 64],
}

pub type PvNode = bool;
pub const PV_NODE: bool = true;
pub const NON_PV_NODE: bool = false;

impl<'a> Searcher<'a> {
    /// Create a new [`Searcher`].
    pub fn best_move(
        position: &Position,
        go_options: &[uci::GoOption],
        stop_signal: Arc<AtomicBool>,
        transposition_table: &'a mut TranspositionTable,
        tablebase: Option<&'static Tablebase>,
    ) -> (ChessMove, Logger) {
        let mut board = position.start_pos;
        let mut root_position = board;
        let mut halfmove_clock = position.starting_halfmove_clock;
        for mv in &position.moves {
            if board.halfmove_reset(*mv) {
                halfmove_clock = 0;
            } else {
                halfmove_clock += 1;
            }
            board.make_move(*mv, &mut root_position);
            board = root_position;
        }

        let mut stack_states = [StackState::default(); Ply::MAX.as_usize() + 1];
        stack_states[0].eval = Some(root_position.evaluate());
        stack_states[0].zobrist = root_position.get_hash();
        stack_states[0].halfmove_clock = halfmove_clock;

        let root_moves: MoveVec = MoveGen::new_legal(&root_position).into();
        let logger = Logger::new().silent(go_options.iter().any(|o| *o == uci::GoOption::Silent));
        let limits = Limits::from(go_options).with_time_control(&root_position);

        let mut searcher = Self {
            root_position,
            ss: stack_states,
            limits,
            logger,
            stop_signal,
            root_moves: root_moves.mvv_lva_rated(&root_position).sorted(),
            transposition_table,
            tablebase,
            history_stats: [[0; 64]; 64],
        };

        let bm = searcher.run();
        (bm, searcher.logger)
    }
}
