use chess::{Board, ChessMove, Piece};

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

        // En passant? Because there now is no piece on dest, if the moving piece is a pawn and it
        // changes file, it must be an en passant capture.
        if board.piece_on(self.get_source()) == Some(Piece::Pawn)
            && self.get_dest().get_file() != self.get_source().get_file()
        {
            return Some(Piece::Pawn);
        }

        None
    }
}
