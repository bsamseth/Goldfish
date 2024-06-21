use chess::{Board, ChessMove, Piece};

/// Extension trait for `Board` to add some utility methods.
#[allow(clippy::module_name_repetitions)]
pub trait BoardExt {
    /// Return true if the move would cause a reset of the 50-move rule counter.
    ///
    /// Will return true if the move is a capture or a pawn move.
    ///
    /// # Panics
    /// Panics if there is no piece on the `from` square.
    fn halfmove_reset(&self, mv: ChessMove) -> bool;

    /// Return true if the board has insufficient material for either side to checkmate.
    ///
    /// This does not include all technically insufficient material situations, but if this
    /// returns true, it is guaranteed that the game is a draw regardless of the moves played.
    fn has_insufficient_material(&self) -> bool;

    /// Return true if the current side to move is in check.
    fn in_check(&self) -> bool;
}

impl BoardExt for Board {
    fn halfmove_reset(&self, mv: ChessMove) -> bool {
        self.piece_on(mv.get_source()).unwrap() == Piece::Pawn
            || self.piece_on(mv.get_dest()).is_some()
    }

    fn has_insufficient_material(&self) -> bool {
        self.pieces(Piece::Pawn).0 == 0
            && self.pieces(Piece::Queen).0 == 0
            && self.pieces(Piece::Rook).0 == 0
            && self.pieces(Piece::Bishop).popcnt() + self.pieces(Piece::Knight).popcnt() <= 1
    }

    fn in_check(&self) -> bool {
        self.checkers().0 != 0
    }
}
