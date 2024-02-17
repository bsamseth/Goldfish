use chess::{Board, ChessMove, MoveGen};

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
    pub fn return_if_no_moves(moves: &MoveGen, board: &Board, ply: Ply) -> Result<(), Value> {
        if moves.len() == 0 {
            if board.in_check() {
                return Err(Value::mated_in(ply));
            }
            return Err(Value::DRAW);
        }
        Ok(())
    }
}
