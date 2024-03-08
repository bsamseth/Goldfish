//! # Speculative cutoffs
//!
//! This module contains functions that determine if a cutoff is likely to be good during the search.
//! All these functions are speculative, to a varying degree. This means that they may not be sound
//! in all situations, but their net impact should still be good.
//!
//! Some functions are tunable, in which case the tunable parameters should be defined in [`crate::tune`].
//!
//! Non-speculative cutoffs are implemented in [`super::cuts`].

use chess::{Board, Piece};

use super::Searcher;
use crate::{
    board::BoardExt,
    newtypes::{Depth, Ply, Value},
    tt::Bound,
    tune,
};

impl Searcher {
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
    pub fn null_move_pruning(
        &mut self,
        board: &Board,
        beta: &mut Value,
        depth: Depth,
        ply: Ply,
    ) -> Result<(), Value> {
        if board.in_check()
            || self.stack_state(ply - Ply::new(1)).null_move
            || Value::CHECKMATE_THRESHOLD <= *beta
            || self.stack_state(ply).eval < *beta
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
        let mut value = -Value::from(self.negamax(
            &board,
            new_depth,
            -*beta,
            -*beta + Value::new(1),
            ply + Ply::new(1),
        ));

        self.undo_null_move(ply);

        if *beta <= value {
            // Don't return unproven mates.
            if Value::CHECKMATE_THRESHOLD <= value {
                value = *beta;
            }

            self.transposition_table.write().unwrap().store(
                self.stack_state(ply).zobrist,
                None,
                Bound::Lower,
                value,
                new_depth + Depth::new(1),
                ply,
            );

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

        let eval = self.stack_state(ply).eval;

        match depth.as_inner() {
            3 if eval + tune::speculate::RAZOR_MARGIN <= alpha => {
                *depth = Depth::new(2);
            }
            2 if eval + tune::speculate::EXTENDED_FUTILITY_MARGIN <= alpha => {
                *depth = Depth::new(1);
            }
            1 if eval + tune::speculate::FUTILITY_MARGIN <= alpha => {
                return Err(Value::from(self.quiescence_search(board, alpha, beta, ply)));
            }
            _ => {}
        }
        Ok(())
    }
}

/// Delta pruning (quiescence search)
///
/// If the static evaluation is sufficiently below alpha, we can assume that the position is
/// losing, and we can prune the node.
///
/// TODO: Make two versions of this: One pre-move generation that checks for greates possible
/// swing, and one that takes a move and checks if the captured piece is enough to swing.
///
pub fn delta_pruning(board: &Board, static_eval: Value, alpha: Value) -> Result<(), Value> {
    if !board.in_check() && static_eval + tune::speculate::DELTA_MARGIN <= alpha {
        return Err(alpha);
    }
    Ok(())
}
