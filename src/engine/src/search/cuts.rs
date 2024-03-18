//! # Cutoffs
//!
//! This module contains functions that determine if a cutoff is possible during the search.
//! All these functions are sound, meaning that they don't rely on speculative heuristics.
//!
//! Speculative cutoffs are implemented in [`super::speculate`].

use chess::{Board, ChessMove, MoveGen};

use fathom::Wdl;

use super::Searcher;
use crate::board::BoardExt;
use crate::evaluate::Evaluate;
use crate::newtypes::{Depth, Ply, Value};
use crate::tt::Bound;

impl Searcher {
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
    pub fn pv_search(
        &mut self,
        board: &Board,
        depth: Depth,
        alpha: Value,
        beta: Value,
        ply: Ply,
        mv_nr: usize,
    ) -> Value {
        if depth > Depth::new(1) && mv_nr > 0 {
            let value = -Value::from(self.negamax(
                board,
                depth - Depth::new(1),
                -alpha - Value::new(1),
                -alpha,
                ply + Ply::new(1),
            ));

            if value <= alpha {
                return value;
            }
        }

        -Value::from(self.negamax(
            board,
            depth - Depth::new(1),
            -beta,
            -alpha,
            ply + Ply::new(1),
        ))
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
    #[inline]
    pub fn return_if_draw(&self, board: &Board, ply: Ply) -> Result<(), Value> {
        if self.is_draw(board, ply) {
            return Err(Value::DRAW);
        }
        Ok(())
    }

    /// Retrieve the best move from the transposition table.
    ///
    /// If the position is in the transposition table, and the stored value is usable at the
    /// current depth, update the alpha and beta bounds and return the move.
    ///
    /// The bounds are updated in place. If the update leaves a zero-width window, the function
    /// signals that an early return is possible.
    #[inline]
    pub fn get_bounds_and_move_from_tt(
        &self,
        alpha: &mut Value,
        beta: &mut Value,
        ply: Ply,
        depth: Depth,
    ) -> Result<Option<ChessMove>, Value> {
        if let Some((mv, bound, value)) =
            self.transposition_table
                .read()
                .unwrap()
                .get(self.stack_state(ply).zobrist, depth, ply)
        {
            if bound & Bound::Lower && *alpha < value {
                *alpha = value;
            }
            if bound & Bound::Upper && value < *beta {
                *beta = value;
            }

            if alpha >= beta {
                return Err(value);
            }

            Ok(mv)
        } else {
            Ok(None)
        }
    }

    /// Adjust alpha/beta if the position is in the tablebase.
    #[inline]
    pub fn check_tablebase(
        &mut self,
        board: &Board,
        alpha: &mut Value,
        beta: &mut Value,
        ply: Ply,
        depth: Depth,
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
                Wdl::CursedWin => (Value::DRAW + Value::new(1), Bound::Exact),
                Wdl::BlessedLoss => (Value::DRAW - Value::new(1), Bound::Exact),
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
                self.transposition_table.write().unwrap().store(
                    self.stack_state(ply).zobrist,
                    None,
                    bound,
                    value,
                    // This could be stored with Depth::MAX, but that would potentially
                    // saturate the tt with only tablebase entries, which is a waste of
                    // a tt. Instead, give it a reasonable depth that should keep it around
                    // for a while, but not forever. Five is a magic number, not tested extensively.
                    depth + Depth::new(5),
                    ply,
                );
                return Err(value);
            }
        }
        Ok(())
    }
}

/// Get a lower bound score of the position.
///
/// If the side to move isn't in check, a lower bound evaluation of the position is the
/// evaluation of the position itself. This is because if there are no better captures
/// available, we could just choose to not capture anything ("stand pat" from poker, meaning to
/// not draw any new cards).
///
/// Should this lower bound be better than beta, this will signal that an early return is
/// possible.
///
/// `alpha` is updated in place if the lower bound is better than the current alpha.
#[inline]
pub fn standing_pat(board: &Board, alpha: &mut Value, beta: Value) -> Result<Value, Value> {
    let mut best_value = -Value::INFINITE;
    if !board.in_check() {
        best_value = board.evaluate();

        if best_value > *alpha {
            *alpha = best_value;

            if best_value >= beta {
                return Err(best_value);
            }
        }
    }
    Ok(best_value)
}

/// Early return if there are no legal moves in the position.
///
/// If the side to move is in check, then the score is negative [`Value::CHECKMATE`].
/// Otherwise, the score is [`Value::DRAW`].
///
/// If there are moves to play, this is a no-op.
#[inline]
pub fn return_if_no_moves(moves: &MoveGen, board: &Board, ply: Ply) -> Result<(), Value> {
    if moves.len() == 0 {
        if board.in_check() {
            return Err(Value::mated_in(ply));
        }
        return Err(Value::DRAW);
    }
    Ok(())
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
    let best_mate = Value::mate_in(ply + Ply::new(1));
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
        && static_eval + crate::evaluate::QUEEN_VALUE + crate::evaluate::QUEEN_VALUE
            - crate::evaluate::PAWN_VALUE
            <= alpha
    {
        return Err(alpha);
    }
    Ok(())
}
