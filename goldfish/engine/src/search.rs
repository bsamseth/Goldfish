mod helpers;
mod minmax;
mod stackstate;

use std::sync::{Arc, RwLock};

use chess::Board;

use super::limits::Limits;
use super::logger::Logger;
use super::movelist::MoveVec;
use super::newtypes::Ply;
use super::stop_signal::StopSignal;
use super::tt::TranspositionTable;
use stackstate::StackState;

#[derive(Debug)]
pub struct Searcher {
    root_position: Board,
    ss: [StackState; Ply::MAX.as_usize() + 1],
    limits: Limits,
    logger: Logger,
    stop_signal: StopSignal,
    root_moves: MoveVec,
    transposition_table: Arc<RwLock<TranspositionTable>>,
}
