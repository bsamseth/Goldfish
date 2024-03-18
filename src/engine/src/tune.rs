//! All tunable parameters should be listed here.
//!
//! This is so that tuning might be done at some point in the future.
//! For now, this just serves as a place to document all "magic numbers" in the engine.

pub mod time_control {
    /// Assume we have to play this many moves until time control/game end.
    pub const DEFAULT_MOVES_TO_GO: usize = 40;

    /// Never spend more than this much time on the clock, as a percentage of the total time left.
    pub const TIME_BUFFER_PERCENTAGE: usize = 95;

    /// If we have less than this much time left, just return a move as soon as possible (milliseconds).
    pub const MIN_SEARCH_TIME: usize = 25;
}

pub mod speculate {
    use crate::newtypes::{Depth, Value};

    /// Razor margin in centipawns.
    pub const RAZOR_MARGIN: Value = Value::new(650);

    /// Extended futility margin in centipawns.
    pub const EXTENDED_FUTILITY_MARGIN: Value = Value::new(500);

    /// Futility margin in centipawns.
    pub const FUTILITY_MARGIN: Value = Value::new(300);

    /// Delta pruning margin in centipawns.
    pub const DELTA_MARGIN: Value = Value::new(250);

    /// Internal iterative deepening depth reduction.
    pub const IID_DEPTH_REDUCTION: Depth = Depth::new(7);
}

pub mod move_ordering {
    /// How many centipawns bonus to give to moves that are the most frequent in history stats.
    ///
    /// Other moves will be given a bonus scaled linearly on how frequent they are compared to the
    /// most frequent move square.
    pub const MAX_HISTORY_STATS_IMPACT: usize = 100;
}
