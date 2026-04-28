//! # Cutoffs
//!
//! This module contains functions that determine if a cutoff is possible during the search.
//! All these functions are sound, meaning that they don't rely on speculative heuristics.
//!
//! Speculative cutoffs are implemented in [`super::speculate`].

use chess::{Board, Piece};

use fathom::Wdl;

use super::{PvNode, Searcher, NON_PV_NODE};
use crate::board::BoardExt;
use crate::evaluate::Evaluate;
use crate::newtypes::{Depth, Ply, Value};
use crate::tt::{self, Bound, EntryWriter};

impl Searcher<'_> {
    /// Principal Variation Search
    ///
    /// Search first move fully, then just check for moves that will improve alpha using a 1-point
    /// search window. If first move was best, then we will save lots of time as bounding is much
    /// faster than finding exact scores. Given a good ordering (which we have due to iterative
    /// deepening) the first move will be very good, and lots of cutoffs can be made.
    ///
    /// If we find a later move that actually improves alpha, we must search this properly to find
    /// its value. The idea is that this drawback is smaller than the improvements gained.
    #[inline]
    pub fn pv_search<const PV: PvNode>(
        &mut self,
        board: &Board,
        depth: Depth,
        alpha: Value,
        beta: Value,
        ply: Ply,
        mv_nr: usize,
    ) -> Value {
        if PV && depth > Depth::ONE && mv_nr > 0 {
            let value = -Value::from(self.negamax::<NON_PV_NODE>(
                board,
                depth.decrement(),
                (-alpha).decrement(),
                -alpha,
                ply.increment(),
            ));

            if value <= alpha {
                return value;
            }
        }

        -Value::from(self.negamax::<PV>(board, depth.decrement(), -beta, -alpha, ply.increment()))
    }

    /// Early return the board evaluation if we are at a forced leaf node.
    ///
    /// A forced leaf node occurs when:
    ///
    ///   - We have reached the maximum ply allowed, or
    ///   - We have been told to stop searching.
    #[inline]
    pub fn return_evaluation_if_at_forced_leaf(
        &self,
        board: &Board,
        ply: Ply,
    ) -> Result<(), Value> {
        if ply == Ply::MAX || self.should_stop() {
            return Err(board.evaluate());
        }
        Ok(())
    }

    /// Early return with [`Value::DRAW`] if the position is a draw.
    /// TODO: check for upcomming draw by repetition and increase alpha if alpha<draw
    /// TODO: Check for draw by insufficient material
    #[inline]
    pub fn return_if_draw(&self, board: &Board, ply: Ply) -> Result<(), Value> {
        if self.is_draw(board, ply) {
            return Err(Value::DRAW);
        }
        Ok(())
    }

    /// Retrieve data from the transposition table if it exists for the position.
    ///
    /// For non-PV nodes, if the position is in the transposition table and the stored value is
    /// usable at the current depth, we may return early.
    ///
    /// The bounds are updated in place. If the update leaves a zero-width window, the function
    /// signals that an early return is possible.
    #[inline]
    pub fn check_tt<const PV: PvNode>(
        &mut self,
        beta: Value,
        ply: Ply,
        depth: Depth,
    ) -> Result<(Option<tt::Data>, tt::EntryWriter), Value> {
        let ss = self.stack_state(ply);
        let halfmove_clock = ss.halfmove_clock;
        let (tt_data, tt_writer) =
            self.transposition_table
                .probe_mut(ss.zobrist, ply, halfmove_clock);

        let Some(tt_data) = tt_data else {
            return Ok((None, tt_writer));
        };

        if !PV && tt_data.value.is_some() {
            let value = tt_data.value.unwrap();

            // If we're not in a PV node, we check for an early cutoff.
            // To do so, the depth of the stored entry must be greater than the depth we are to
            // search. If the cutoff doesn't cause a fail-high, we also accept entries that are
            // of equal depth.
            let required_depth = Depth::new(depth.as_inner() - i16::from(value <= beta));
            let required_bound = if value >= beta {
                Bound::Lower
            } else {
                Bound::Upper
            };

            if tt_data.depth > required_depth
                && tt_data.bound & required_bound
                // Partial workaround for the graph history interaction problem
                // For high rule50 counts don't produce transposition table cutoffs.
                && halfmove_clock < 90
            {
                return Err(tt_data.value.unwrap());
            }
        }

        Ok((Some(tt_data), tt_writer))
    }

    /// Adjust alpha/beta if the position is in the tablebase.
    #[inline]
    pub fn check_tablebase<const PV: PvNode>(
        &mut self,
        board: &Board,
        alpha: &mut Value,
        beta: &mut Value,
        ply: Ply,
        depth: Depth,
        writer: &EntryWriter,
    ) -> Result<(), Value> {
        if let Some(wdl) = self
            .tablebase
            .and_then(|tb| tb.probe_wdl(board, self.stack_state(ply).halfmove_clock))
        {
            self.logger.tb_hit();
            let (value, bound) = match wdl {
                Wdl::Win => (Value::known_win_in(ply), Bound::Lower),
                Wdl::Loss => (Value::known_loss_in(ply), Bound::Upper),
                Wdl::Draw => (Value::DRAW, Bound::Exact),
                // Treat draws by rule 50 as draws, but slightly better/worse than normal
                // draws. This way, a cursed win is still better than a draw (because the
                // opponent might not play optimally), and a blessed loss is still worse than a
                // normal draw (because we'd rather not be reliant on rule 50 to hold a
                // draw).
                Wdl::CursedWin => (Value::DRAW.increment(), Bound::Exact),
                Wdl::BlessedLoss => (Value::DRAW.decrement(), Bound::Exact),
            };

            // Apply the tb bounds to alpha/beta:
            if bound & Bound::Lower && value > *alpha {
                *alpha = value;
            }
            if bound & Bound::Upper && value < *beta {
                *beta = value;
            }

            // If the updated bounds leave a zero-width window, signal that an early return is
            // possible. Exact bounds always leave a zero-width window.
            if alpha >= beta {
                // SAFETY: The tt writer was produced in this search frame, so the entry is still
                // valid at this point.
                unsafe {
                    writer.save::<PV>(&tt::EntryWriterOpts {
                        bound,
                        depth: Depth::new(depth.as_inner().saturating_add(7) - 1),
                        value: Some(value),
                        ply,
                        ..Default::default()
                    });
                }
                return Err(value);
            }
        }
        Ok(())
    }
}

/// Get a lower bound score and evaluaiton of the position, if not in check.
///
/// If the side to move is in check, no evaluaiton is produced and the lower bound is negative [`Value::INFINITE`].
///
/// If the side to move isn't in check, a lower bound evaluation of the position is the evaluation
/// of the position itself. This is because if there are no better captures available, we could
/// just choose to not capture anything ("stand pat" from poker, meaning to not draw any new
/// cards). Assumes we are not in zugzwang, which, com'on, when does that ever happen?!.
#[inline]
//pub fn standing_pat(board: &Board, tt_data: &tt::Data) -> (Value, Option<Value>) {
pub fn lower_bound_eval(board: &Board, tt_data: Option<&tt::Data>) -> (Value, Option<Value>) {
    if board.in_check() {
        return (-Value::INFINITE, None);
    }

    let eval = tt_data
        .as_ref()
        .and_then(|tt_data| tt_data.eval)
        .unwrap_or_else(|| board.evaluate());
    let mut best_value = eval;

    // Use tt entries to improve the evaluation.
    if let Some(bounded_eval) = tt_data.as_ref().and_then(|t| t.bounded(best_value)) {
        if !bounded_eval.is_known_result() {
            best_value = bounded_eval;
        }
    }

    (best_value, Some(eval))
}

/// Mate distance pruning
///
/// Even if we mate at the next move our score would be at best CHECKMATE - ply, but if alpha
/// is already bigger because a shorter mate was found upward in the tree then there is no need
/// to search because we will never beat the current alpha. Same logic but with reversed signs
/// applies also in the opposite condition of being mated instead of giving mate.
///
/// This may modify `alpha` and `beta` in place. If `alpha >= beta` after this, the function
/// signals that an early return is possible.
#[inline]
pub fn mate_distance_pruning(alpha: &mut Value, beta: &mut Value, ply: Ply) -> Result<(), Value> {
    let worst_mate = Value::mated_in(ply);
    let best_mate = Value::mate_in(ply.increment());
    if *alpha < worst_mate {
        *alpha = worst_mate;
    }
    if *beta > best_mate {
        *beta = best_mate;
    }
    if alpha >= beta {
        return Err(*alpha);
    }
    Ok(())
}

/// Full delta pruning (quiescence search)
///
/// If the static evaluation is sufficiently below alpha so that any possible single move will not
/// be enough to increase alpha, we can safely prune the entire node.
///
/// This can be called before move generation during quiescence search.
///
/// The greatest possible material swing is capturing a queen with a pawn while promoting.
#[inline]
pub fn full_delta_pruning(board: &Board, static_eval: Value, alpha: Value) -> Result<(), Value> {
    if !board.in_check()
        && static_eval
            + crate::evaluate::piece_value(Piece::Queen)
            + crate::evaluate::piece_value(Piece::Queen)
            - crate::evaluate::piece_value(Piece::Pawn)
            <= alpha
    {
        return Err(alpha);
    }
    Ok(())
}
