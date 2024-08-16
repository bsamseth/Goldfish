use chess::{Board, ChessMove, Color, Piece};

/// Extension trait for [`ChessMove`] to add some utility methods.
#[allow(clippy::module_name_repetitions)]
pub trait ChessMoveExt {
    fn captures(&self, board: &Board) -> Option<Piece>;
}

impl ChessMoveExt for ChessMove {
    fn captures(&self, board: &Board) -> Option<Piece> {
        if let Some(piece) = board.piece_on(self.get_dest()) {
            return Some(piece);
        }

        // En passant?
        if let Some(ep) = board.en_passant() {
            // The ep stored in board is the pawn that may be captured. We want to test if this
            // move takes it, so we shift the ep down/up a rank.
            let ep = if board.side_to_move() == Color::White {
                ep.up()
            } else {
                ep.down()
            };
            // SAFETY: En-passant squares are on the 4th or 5th rank, so they have an up/down.
            let ep = unsafe { ep.unwrap_unchecked() };
            if self.get_dest() == ep && board.piece_on(self.get_source()) == Some(Piece::Pawn) {
                return Some(Piece::Pawn);
            }
        }

        None
    }
}
