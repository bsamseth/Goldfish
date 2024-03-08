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
