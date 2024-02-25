mod chess_to_fathom;
mod root_probe;

use std::path::Path;
use std::sync::Mutex;

use fathom_syzygy::Fathom;
pub use fathom_syzygy::Wdl;

use root_probe::RootMovesFilter;

/// An endgame tablebase.
///
/// This struct is a wrapper around the Fathom syzygy tablebase library. It provides a simple
/// interface for probing the tablebase, and is thread-safe.
#[derive(Debug)]
pub struct Tablebase {
    // The mutex is needed for thread safety, but we put this mutex internally.
    // This is as opposed to the transposition table, and is done so that the check for whether a
    // position is in the tablebase can be done quickly, without needing to lock the mutex.
    // Only when we actually need to probe the tablebase do we lock the mutex.
    fathom: Mutex<Fathom>,
    max_pieces: u32,
}

impl Tablebase {
    pub fn new<P: AsRef<Path>>(path: P) -> Result<Self, fathom_syzygy::Error> {
        let path = path.as_ref();
        let fathom = Fathom::new(path)?;
        let max_pieces = fathom.max_pieces();
        let tb = Self {
            fathom: Mutex::new(fathom),
            max_pieces,
        };

        tracing::debug!(
            ?path,
            max_pieces = tb.fathom.lock().unwrap().max_pieces(),
            "Tablebase loaded successfully"
        );

        Ok(tb)
    }

    /// Check the root position against the tablebase.
    ///
    /// If the position is not in the tablebase, this returns `None` quickly. Otherwise, it returns
    /// the [`RootProbeResult`] which contains the win-draw-loss value for the position, as well as
    /// the capability to filter a [`MoveVec`] to only include moves that preserve the
    /// win-draw-loss result.
    pub fn probe_root<'a>(
        &'a self,
        board: &'a chess::Board,
        halfmove_clock: usize,
    ) -> Option<RootMovesFilter<'a>> {
        if !self.board_in_tablebase(board) {
            return None;
        }

        let result = self
            .fathom
            .lock()
            .unwrap()
            .get_probers()
            .0
            .probe(&chess_to_fathom::fathom_position(board, halfmove_clock))?;

        Some(RootMovesFilter::new(
            result.wdl,
            self,
            board,
            halfmove_clock,
        ))
    }

    /// Check the win-draw-loss value for the position.
    ///
    /// This function is quick to call if the position is not in the tablebase.
    pub fn probe_wdl(&self, board: &chess::Board, halfmove_clock: usize) -> Option<Wdl> {
        if !self.board_in_tablebase(board) {
            return None;
        }

        self.fathom
            .lock()
            .unwrap()
            .get_probers()
            .1
            .probe(&chess_to_fathom::fathom_position(board, halfmove_clock))
    }

    /// Quickly check if the board should be in the tablebase.
    fn board_in_tablebase(&self, board: &chess::Board) -> bool {
        board.combined().popcnt() <= self.max_pieces
    }
}
