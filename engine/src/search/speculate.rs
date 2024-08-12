//! # Speculative cutoffs
//!
//! This module contains functions that determine if a cutoff is likely to be good during the search.
//! All these functions are speculative, to a varying degree. This means that they may not be sound
//! in all situations, but their net impact should still be good.
//!
//! Some functions are tunable, in which case the tunable parameters should be defined in [`crate::tune`].
//!
//! Non-speculative cutoffs are implemented in [`super::cuts`].

use chess::{Board, ChessMove, Piece};

use super::PvNode;
use super::{Searcher, NON_PV_NODE};
use crate::chessmove::ChessMoveExt;
use crate::opts::OPTS;
use crate::{
    board::BoardExt,
    newtypes::{Depth, Ply, Value},
    tt,
};

impl Searcher<'_> {
    /// Null move pruning.
    ///
    /// If we have a position that is so strong that even if we don't move (i.e. pass), we still
    /// exceed `beta`, then we can prune this node. To make this worthwhile and somewhat safe,
    /// we can do a reduced depth search after passing. If this reduced depth search evaluates our
    /// position to be better than `beta`, we assume it is safe to prune.
    ///
    /// We only do this we have reason to belive that NMP will be useful, which  we do by testing
    /// for `eval >= beta`.
    ///
    /// Further, for this to be safe, the following must _NOT_ be true:
    ///
    ///  1. We are in check (NM would be illegal).
    ///  2. The last move made was a null move (double null move has no effect other than to reduce depth).
    ///  3. A beta-cutoff must be by finding a mate score (mates with NM is not proven).
    ///  4. We are in zugzwang (not moving is better than any other move).
    ///
    /// Number 4 is hard to guarantee (but possible with verification search, see stockfish). But
    /// by not using null move when we only have K and P we escape most cases.
    ///
    /// # Assumptions
    /// The evaluation of the position should already be available in the stack state before
    /// calling this function.
    #[inline]
    pub fn null_move_pruning(
        &mut self,
        board: &Board,
        beta: &mut Value,
        depth: Depth,
        ply: Ply,
    ) -> Result<(), Value> {
        if board.in_check()
            || self.stack_state(ply.decrement()).null_move
            || beta.is_known_result()
            || self.stack_state(ply).eval.is_some_and(|eval| eval < *beta)
            || ((board.pieces(Piece::Queen)
                | board.pieces(Piece::Rook)
                | board.pieces(Piece::Bishop)
                | board.pieces(Piece::Knight))
                & board.color_combined(board.side_to_move()))
            .0 == 0
        {
            return Ok(());
        }

        let board = self.make_null_move(board, ply);

        // New depth = depth - 3.
        // This is so that 1) it's the other side's turn in sibling nodes, and 2) this allows
        // recursive null moves.
        let new_depth = Depth::new(depth.as_inner().saturating_sub(3));
        let value = -Value::from(self.negamax::<NON_PV_NODE>(
            &board,
            new_depth,
            -*beta,
            (-*beta).increment(),
            ply.increment(),
        ));

        self.undo_null_move(ply);

        // Don't return unproven wins.
        if value >= *beta && !value.is_known_win() {
            return Err(value);
        }

        Ok(())
    }

    /// Futility pruning
    ///
    /// This combines futilit pruning, extended futility pruning and razoring into one go.
    /// The same idea goes for all three: If the static evaluation is sufficiently below alpha, we
    /// can reduce the depth by 1 and continue. In the case of futility pruning, we skip straight
    /// to quiescence search.
    ///
    /// Source: <http://www.frayn.net/beowulf/theory.html/>
    #[inline]
    pub fn futility_pruning(
        &mut self,
        board: &Board,
        alpha: Value,
        beta: Value,
        depth: &mut Depth,
        ply: Ply,
    ) -> Result<(), Value> {
        if board.in_check() {
            return Ok(());
        }

        let eval = self
            .stack_state(ply)
            .eval
            .expect("Evaluation should be available");

        match depth.as_inner() {
            3 if eval + OPTS.razor_margin <= alpha => {
                *depth = Depth::new(2);
            }
            2 if eval + OPTS.extended_futility_margin <= alpha => {
                *depth = Depth::new(1);
            }
            1 if eval + OPTS.futility_margin <= alpha => {
                return Err(Value::from(
                    self.quiescence_search::<NON_PV_NODE>(board, alpha, beta, ply),
                ));
            }
            _ => {}
        }
        Ok(())
    }

    /// Internal iterative deepening
    ///
    /// When we have no good guess for the best move, do a reduced search first to find a likely
    /// candidate. Only do this if a search would lead to a new entry in the TT.
    #[inline]
    pub fn internal_iterative_deepening<const PV: PvNode>(
        &mut self,
        board: &Board,
        tt_move: Option<ChessMove>,
        depth: Depth,
        alpha: Value,
        beta: Value,
        ply: Ply,
    ) -> Option<ChessMove> {
        // TODO: Testing indicates this is a wash compared to not doing it. Consider dropping it.
        if PV && tt_move.is_none() && depth >= OPTS.iid_depth_lower_bound {
            let depth = depth - OPTS.iid_depth_reduction;
            let _ = self.negamax::<PV>(board, depth, alpha, beta, ply);
            let ss = self.stack_state(ply);
            if let Some(tt::Data { mv: Some(mv), .. }) =
                self.transposition_table
                    .probe(ss.zobrist, ply, ss.halfmove_clock)
            {
                tracing::warn!("IID found a move");
                return Some(mv);
            }
            tracing::warn!("IID failed to find a move");
        }
        tt_move
    }
}

/// Delta pruning (quiescence search)
///
/// If the static evaluation is so low that the move is unlikely to improve alpha, we can skip the move.
///
/// Returns true if the move should be skipped.
///
/// # Panics
/// This assumes that if the position is not in check, then the move is a capture. This should be
/// the case during quiescence search.
#[inline]
pub fn delta_pruning(board: &Board, mv: ChessMove, static_eval: Value, alpha: Value) -> bool {
    !board.in_check()
        && mv.get_promotion().is_none()
        && static_eval
            + crate::evaluate::piece_value(mv.captures(board).unwrap())
            + OPTS.delta_margin
            <= alpha
}
