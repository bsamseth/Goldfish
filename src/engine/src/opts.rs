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
    pub tc_time_buffer_percentage: usize,
    #[uci(default = "25", kind = "spin", min = "0")]
    pub tc_min_search_time_ms: usize,

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

mod singleton {
    use super::Opts;

    /// Singleton instance of the [`Opts`] struct.
    ///
    /// This is effectively a [`std::sync::atomic::AtomicPtr<Opts>`], but without the atomic overhead.
    /// It is safe to use as long as the following usage pattern is followed:
    ///
    ///   1. The `store` method is called only once, and only from a single thread.
    ///      This should be at startup of the UCI loop, where the program should store
    ///      a pointer to a newly created `Opts` struct stored in [`crate::Engine`]
    ///   2. The contents of the options may only ever be changed while they are not being used
    ///      anywhere else. I.e. responding to a `setoption` command may only be done while no
    ///      search is running.
    ///   3. Provided 1 and 2 are upheld, the instance may be used to read option values during the
    ///      search without any synchronization.
    ///
    ///  [`OptPtr`] implements [`std::ops::Deref<Target=Opts>`], so it can be used as a reference to the options.
    ///  It does not implement [`std::ops::DerefMut`], as writing to the options should be done with care, and
    ///  while respecting the above rules.
    ///   
    pub(crate) static OPTS: OptPtr = OptPtr::new();

    pub struct OptPtr {
        p: std::cell::UnsafeCell<*mut Opts>,
    }
    impl OptPtr {
        pub const fn new() -> Self {
            OptPtr {
                p: std::cell::UnsafeCell::new(std::ptr::null_mut()),
            }
        }
        pub unsafe fn store(&self, opts: *mut Opts) {
            unsafe {
                *self.p.get() = opts;
            }
        }
        pub unsafe fn get(&self) -> *mut Opts {
            unsafe { *self.p.get() }
        }
    }
    impl std::ops::Deref for OptPtr {
        type Target = Opts;
        fn deref(&self) -> &Self::Target {
            unsafe { &*self.get() }
        }
    }
    // Safety: These are only "safe" when the above rules are followed. So this is a lie.
    unsafe impl Send for OptPtr {}
    unsafe impl Sync for OptPtr {}
}

pub(crate) use singleton::OPTS;
