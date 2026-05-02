use crate::Piece;
use crate::Square;

/// Representation of a chess move.
#[derive(Debug, Clone, Copy)]
pub struct ChessMove {
    source: Square,
    destination: Square,
    promotion: Option<Piece>,
}

impl ChessMove {
    /// Create a new move.
    #[inline]
    pub fn new(source: Square, dest: Square, promotion: Option<Piece>) -> ChessMove {
        ChessMove {
            source: source,
            destination: dest,
            promotion: promotion,
        }
    }

    /// Return the square that the moving piece starts on.
    #[inline]
    pub fn source(&self) -> Square {
        self.source
    }

    /// Return the square that the moving piece lands on.
    #[inline]
    pub fn destination(&self) -> Square {
        self.destination
    }

    /// Return the promotion piece, if this is a promotion move.
    #[inline]
    pub fn get_promotion(&self) -> Option<Piece> {
        self.promotion
    }
}
