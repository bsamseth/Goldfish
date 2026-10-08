use crate::{Bitboard, Color, Piece, Square};

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
    pub fn new(source: Square, destination: Square, promotion: Option<Piece>) -> ChessMove {
        ChessMove {
            source,
            destination,
            promotion,
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

impl ChessMove {
    /// Return all the possible destination squares for a king move from `sq`.
    ///
    /// This does not consider whether or not the implied move(s) would be legal in a particular
    /// game scenario. It also does not consider if the desitnation square has a friendly piece on it.
    pub fn king_moves_from(sq: Square) -> Bitboard {
        Bitboard(crate::generated_tables::KING_MOVES[sq.as_index()])
    }
    /// Return all the possible destination squares for a knight move from `sq`.
    ///
    /// This does not consider whether or not the implied move(s) would be legal in a particular
    /// game scenario. It also does not consider if the desitnation square has a friendly piece on it.
    pub fn knight_moves_from(sq: Square) -> Bitboard {
        Bitboard(crate::generated_tables::KNIGHT_MOVES[sq.as_index()])
    }
    pub fn pawn_moves_from(sq: Square, color: Color, blockers: Bitboard) -> Bitboard {
        Bitboard(crate::generated_tables::pawn[sq.as_index()])
    }
}
