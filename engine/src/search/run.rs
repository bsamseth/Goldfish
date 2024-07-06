use chess::ChessMove;

use super::Searcher;
use crate::newtypes::{Depth, Ply, Value};

impl Searcher<'_> {
    /// Run a search and return the best move.
    ///
    /// The search may stop for a variety of reasons, depending on the options set by the user,
    /// or if the user sends a stop signal. Still, it should always eventually return.
    pub fn run(&mut self) -> ChessMove {
        assert!(
            !self.root_moves.is_empty(),
            "no legal moves in starting position, uci-crate promise violation"
        );

        self.filter_root_moves_using_tb();

        // If there's only one legal move, we don't need to search.
        // A general purpose UCI engine should actually search the position anyway, in order to
        // determine the actual evaluation of the position. This, however, is an engine meant to
        // play, and searching a position with only one legal move is a waste of time.
        if self.root_moves.len() == 1 {
            return self.root_moves[0].mv;
        }

        let alpha = if let Some(mate_distance) = self.limits.mate {
            Value::mate_in(mate_distance + Ply::ONE)
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
        self.logger.force_send_status();
        self.root_moves[0].mv
    }
}
