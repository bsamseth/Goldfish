use std::path::PathBuf;

use anyhow::{Context, Result};

use uci::UciOptions;

use crate::newtypes::{Depth, Value};

#[derive(Debug, UciOptions)]
pub struct Opts {
    // Basic options:
    #[uci(
        name = "Hash",
        kind = "spin",
        default = "16",
        min = "1",
        max = "33554432"
    )]
    pub hash_size_mb: usize,
    #[uci(name = "SyzygyPath", kind = "string")]
    pub syzygy_path: Option<PathBuf>,

    // Time control options:
    #[uci(default = "40", kind = "spin", min = "1", max = "100")]
    pub tc_default_moves_to_go: usize,
    #[uci(default = "95", kind = "spin", min = "1", max = "100")]
    pub tc_time_buffer_percentage: u32,
    #[uci(default = "25", kind = "spin", min = "0")]
    tc_min_search_time_ms: u32,

    // Search speculation options:
    //
    /// Razor margin in centipawns.
    #[uci(default = "650", kind = "spin", min = "0")]
    pub razor_margin: Value,
    /// Extended futility margin in centipawns.
    #[uci(default = "500", kind = "spin", min = "0")]
    pub extended_futility_margin: Value,
    /// Futility margin in centipawns.
    #[uci(default = "300", kind = "spin", min = "0")]
    pub futility_margin: Value,
    /// Delta pruning margin in centipawns.
    #[uci(default = "250", kind = "spin", min = "0")]
    pub delta_margin: Value,
    /// Internal iterative deepening depth reduction.
    #[uci(default = "7", kind = "spin", min = "0")]
    pub iid_depth_reduction: Depth,

    // Move ordering options:
    //
    /// How many centipawns bonus to give to moves that are the most frequent in history stats.
    /// Other moves will be given a bonus scaled linearly on how frequent they are compared to the
    /// most frequent move square.
    #[uci(default = "30", kind = "spin", min = "0", max = "200")]
    pub max_history_stats_impact: usize,

    // Evaluation options:
    //
    /// Bonus for having a bishop pair, in centipawns.
    #[uci(default = "10", kind = "spin", min = "0", max = "200")]
    pub bishop_pair_bonus: i32,
}
