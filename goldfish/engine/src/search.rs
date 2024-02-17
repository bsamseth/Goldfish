mod cuts;
mod helpers;
mod negamax;
mod quiescence;
mod root;
mod stackstate;

use std::sync::{Arc, RwLock};

use chess::{Board, ChessMove};

use super::limits::Limits;
use super::logger::Logger;
use super::movelist::MoveVec;
use super::newtypes::{Depth, Ply, Value};
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

impl Searcher {
    /// Run a search and return the best move.
    ///
    /// The search may stop for a variety of reasons, depending on the options set by the user,
    /// or if the user sends a stop signal. Still, it should always eventually return.
    pub fn run(&mut self) -> ChessMove {
        assert!(
            !self.root_moves.is_empty(),
            "no legal moves in starting position, uci-crate promise violation"
        );

        let alpha = if let Some(mate_distance) = self.limits.mate {
            Value::mate_in(mate_distance + Ply::new(1))
        } else {
            -Value::INFINITE
        };

        for depth in 1..=self.limits.depth.as_inner() {
            let depth = Depth::new(depth);

            if self.should_stop() {
                break;
            }

            self.logger.set_current_depth(depth);
            self.search_root(depth, alpha, Value::INFINITE);

            self.root_moves.sort();

            // If we're in mate search mode, and we've found a mate, we can stop if the mate is
            // within the distance we're looking for.
            if let Some(mate) = self.limits.mate {
                if self.root_moves[0].value >= Value::mate_in(mate) {
                    break;
                }
            }
        }

        self.root_moves[0].mv
    }
}
