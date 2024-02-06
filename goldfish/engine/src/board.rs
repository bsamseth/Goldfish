use chess::{Board, ChessMove};

/// Extension trait for `Board` to add some utility methods.
pub trait BoardExt {
    /// Return true if the move would cause a reset of the 50-move rule counter.
    ///
    /// Will return true if the move is a capture or a pawn move.
    ///
    /// # Panics
    /// Panics if there is no piece on the `from` square.
    fn halfmove_reset(&self, mv: ChessMove) -> bool;

    /// Make a chess move onto an already allocated `Board`, and update the halfmove clock.
    ///
    /// If the move is a capture or a pawn move, the halfmove clock will be reset to 0.
    /// Otherwise, it will be incremented by 1.
    /// The `halfmove_clock` parameter is updated in place.
    ///
    /// # Panics
    /// Panics if there is no piece on the `from` square.
    fn make_move_with_halfmove(
        &self,
        mv: ChessMove,
        result: &mut Board,
        halfmove_clock: &mut usize,
    );
}

impl BoardExt for Board {
    fn halfmove_reset(&self, mv: ChessMove) -> bool {
        self.piece_on(mv.get_source()).unwrap() == chess::Piece::Pawn
            || self.piece_on(mv.get_dest()).is_some()
    }

    fn make_move_with_halfmove(
        &self,
        mv: ChessMove,
        result: &mut Board,
        halfmove_clock: &mut usize,
    ) {
        if self.halfmove_reset(mv) {
            *halfmove_clock = 0;
        } else {
            *halfmove_clock += 1;
        }
        self.make_move(mv, result);
    }
}
