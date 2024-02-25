use std::path::Path;

use fathom_syzygy::Fathom;

pub use fathom_syzygy::Wdl;

use crate::movelist::MoveVec;

#[derive(Debug)]
pub struct Tablebase {
    fathom: Fathom,
}

impl Tablebase {
    pub fn new<P: AsRef<Path>>(path: P) -> Result<Self, fathom_syzygy::Error> {
        let path = path.as_ref();
        let tb = Self {
            fathom: Fathom::new(path)?,
        };

        tracing::debug!(
            ?path,
            max_pieces = tb.fathom.max_pieces(),
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
    pub fn probe_root(
        &self,
        board: &chess::Board,
        halfmove_clock: usize,
    ) -> Option<RootProbeResult> {
        todo!()
    }

    /// Check the win-draw-loss value for the position.
    ///
    /// This function is quick to call if the position is not in the tablebase.
    pub fn probe_wdl(&self, board: &chess::Board, halfmove_clock: usize) -> Option<Wdl> {
        todo!()
    }
}

pub struct RootProbeResult {
    pub wdl: Wdl,
    board: chess::Board,
}

impl RootProbeResult {
    /// Filter the provided `moves` to only include moves that preserve the [`Wdl`] value.
    pub fn filter_moves(&self, moves: &mut MoveVec) {
        todo!()
    }
}
