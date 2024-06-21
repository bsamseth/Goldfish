use chess::{Board, ChessMove, Piece, Square};

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
        if board.piece_on(self.get_source()) == Some(Piece::Pawn)
            && self.get_dest().get_file() != self.get_source().get_file()
        {
            return board.piece_on(Square::make_square(
                self.get_source().get_rank(),
                self.get_dest().get_file(),
            ));
        }

        None
    }
}
