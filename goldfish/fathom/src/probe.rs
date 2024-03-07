use chess::{Board, CastleRights, ChessMove, Color, Piece, Rank, Square};

use super::{sys, Tablebase, Wdl};

impl Tablebase {
    /// Probe the Win-Draw-Loss result of the given position.
    ///
    /// If the position is not in the tablebase, this function will return `None` quickly. As such
    /// it can be called during search without any pre-checks.
    ///
    /// # Safety
    /// While not marked as `unsafe`, this function's result is undefined if the board does not
    /// represent a valid position.
    #[must_use]
    #[inline]
    pub fn probe_wdl(&self, board: &Board, halfmove_clock: usize) -> Option<Wdl> {
        if halfmove_clock > 0
            || board.castle_rights(Color::White) != CastleRights::NoRights
            || board.castle_rights(Color::Black) != CastleRights::NoRights
            || (board.color_combined(Color::White) | board.color_combined(Color::Black)).popcnt()
                > self.max_pieces
        {
            return None;
        }

        // Safety: The `Board` should always be valid (a prerequisite for this library to work at all).
        // In order to call this function, a reference to `self` is required, which means the
        // tablebase must have been initialized. Hence it is be safe to call this.
        let result = unsafe {
            sys::tb_probe_wdl(
                board.color_combined(Color::White).0,
                board.color_combined(Color::Black).0,
                board.pieces(Piece::King).0,
                board.pieces(Piece::Queen).0,
                board.pieces(Piece::Rook).0,
                board.pieces(Piece::Bishop).0,
                board.pieces(Piece::Knight).0,
                board.pieces(Piece::Pawn).0,
                ep_to_fathom(board),
                u8::from(board.side_to_move() == Color::White),
            )
        };

        Wdl::try_from(result).ok()
    }

    /// Probe the Distance-To-Zero result of the given position, and return a move filter.
    ///
    /// If the position is in the tablebase, the returned move filter will evaluate to `true` only
    /// for moves that preserve the WDL result of the position.
    ///
    ///
    /// # Safety
    /// While not marked as `unsafe`, this function's result is undefined if the board does not
    /// represent a valid position. It is also _not_ thread-safe.
    ///
    /// TODO: Make this non-thread safety compiler enforced.
    #[must_use]
    pub fn probe_dtz(
        &self,
        board: &Board,
        halfmove_clock: usize,
    ) -> Option<(Wdl, impl Fn(&ChessMove) -> bool)> {
        if board.castle_rights(Color::White) != CastleRights::NoRights
            || board.castle_rights(Color::Black) != CastleRights::NoRights
            || (board.color_combined(Color::White) | board.color_combined(Color::Black)).popcnt()
                > self.max_pieces
        {
            return None;
        }

        let mut results_per_move = [0u32; sys::TB_MAX_MOVES as usize];

        let result = unsafe {
            sys::tb_probe_root(
                board.color_combined(Color::White).0,
                board.color_combined(Color::Black).0,
                board.pieces(Piece::King).0,
                board.pieces(Piece::Queen).0,
                board.pieces(Piece::Rook).0,
                board.pieces(Piece::Bishop).0,
                board.pieces(Piece::Knight).0,
                board.pieces(Piece::Pawn).0,
                u32::try_from(halfmove_clock).ok()?,
                ep_to_fathom(board),
                u8::from(board.side_to_move() == Color::White),
                results_per_move.as_mut_ptr(),
            )
        };

        if result == sys::TB_RESULT_FAILED
            || result == sys::TB_RESULT_STALEMATE
            || result == sys::TB_RESULT_CHECKMATE
        {
            return None;
        }

        let wdl = Wdl::try_from(result).ok()?;

        let results_per_move = results_per_move
            .iter()
            .take_while(|&&r| r != sys::TB_RESULT_FAILED)
            .map(|&r| {
                (
                    Wdl::try_from(sys::tb_get_wdl(r)).unwrap_or_else(|()| unreachable!()),
                    ChessMove::new(
                        unsafe { Square::new(sys::tb_get_from(r).clamp(0, 63) as u8) },
                        unsafe { Square::new(sys::tb_get_to(r).clamp(0, 63) as u8) },
                        match sys::tb_get_promotes(r) {
                            sys::TB_PROMOTES_QUEEN => Some(Piece::Queen),
                            sys::TB_PROMOTES_ROOK => Some(Piece::Rook),
                            sys::TB_PROMOTES_BISHOP => Some(Piece::Bishop),
                            sys::TB_PROMOTES_KNIGHT => Some(Piece::Knight),
                            sys::TB_PROMOTES_NONE => None,
                            _ => unreachable!(),
                        },
                    ),
                )
            })
            .collect::<Vec<_>>();

        let move_filter = move |mv: &ChessMove| -> bool {
            // Look for the move in the results:
            results_per_move
                .iter()
                .any(|(probe_wdl, probe_mv)| *probe_wdl == wdl && probe_mv == mv)
        };

        Some((wdl, move_filter))
    }
}

/// Return the representation of the boards en passant square (if any) that fathom expects.
///
/// The chess crate decided that `board.en_passant()` returns the square of the piece
/// that can be captured via en passant, instead of the en passant square. That is,
///
/// ```
/// let board: chess::Board = "8/8/8/k1pPp3/8/K7/8/8 w - e6 0 1".parse().unwrap();
/// assert!(board.en_passant().unwrap() == chess::Square::E5);
/// ```
/// Compensate for this by shifting up or down a rank, before finally converting to a u32.
#[inline]
fn ep_to_fathom(board: &Board) -> u32 {
    u32::from(
        board
            .en_passant()
            .map_or(Square::A1, |sq| {
                if sq.get_rank() == Rank::Fifth {
                    sq.up().expect("is some because rank is fifth")
                } else {
                    sq.down().expect("is some because rank is fourth")
                }
            })
            .to_int(),
    )
}
