use chess::{Board, ChessMove, MoveGen};

use super::Searcher;
use crate::board::BoardExt;
use crate::evaluate::Evaluate;
use crate::movelist::{MoveEntry, MoveVec};
use crate::newtypes::{Depth, Ply, Value};
use crate::tt::Bound;

impl Searcher {
    /// Run a search and return the best move.
    ///
    /// The search may stop for a variety of reasons, depending on the options set by the user,
    /// or if the user sends a stop signal. Still, it should always eventually return.
    pub fn run(&mut self) -> ChessMove {
        assert!(
            !self.root_moves.is_empty(),
            "no legal moves in starting position, uci-crate promise violation"
        );

        let alpha = if let Some(mate_distance) = self.limits.mate {
            Value::mate_in(mate_distance + Ply::new(1))
        } else {
            -Value::INFINITE
        };

        for depth in 1..=self.limits.depth.as_inner() {
            let depth = Depth::new(depth);

            if self.should_stop() {
                break;
            }

            self.logger.set_current_depth(depth);
            self.search_root(depth, alpha, Value::INFINITE);

            self.root_moves.sort();

            // If we're in mate search mode, and we've found a mate, we can stop if the mate is
            // within the distance we're looking for.
            if let Some(mate) = self.limits.mate {
                if self.root_moves[0].value >= Value::mate_in(mate) {
                    break;
                }
            }
        }

        self.root_moves[0].mv
    }

    fn search_root(&mut self, depth: Depth, mut alpha: Value, beta: Value) {
        assert!(depth > Depth::new(0), "depth must be greater than 0");

        // Reset all values, so the best move as determined at this new (larger) depth is pushed to the front
        self.root_moves.iter_mut().for_each(|m| {
            *m = MoveEntry::new(m.mv);
        });

        let root_board = self.root_position;
        let mut board = root_board;

        for (mv_nr, mv) in self.root_moves.clone().iter().enumerate() {
            self.logger.set_current_move(mv.mv, mv_nr + 1);
            self.logger.send_status();

            self.make_move(&root_board, mv.mv, &mut board, Ply::new(0));

            let value = self.pv_search(&board, depth, alpha, beta, Ply::new(0), mv_nr);

            if self.should_stop() {
                // If we're stopping, we don't trust the value, because it was likely cut off.
                // Rely on whatever we've found so far on only.
                return;
            }

            if value > alpha {
                alpha = value;

                self.root_moves[mv_nr].value = value;

                if value >= beta {
                    return;
                }

                self.logger
                    .send_move(&self.root_moves[mv_nr], &self.build_pv(depth));
            }
        }
    }

    /// Principal Variation Search
    ///
    /// Search first move fully, then just check for moves that will improve alpha using a 1-point
    /// search window. If first move was best, then we will save lots of time as bounding is much
    /// faster than finding exact scores. Given a good ordering (which we have due to iterative
    /// deepening) the first move will be very good, and lots of cutoffs can be made.
    ///
    /// If we find a later move that actually improves alpha, we must search this properly to find
    /// its value. The idea is that this drawback is smaller than the improvements gained.
    fn pv_search(
        &mut self,
        board: &Board,
        depth: Depth,
        alpha: Value,
        beta: Value,
        ply: Ply,
        mv_nr: usize,
    ) -> Value {
        if depth > Depth::new(1) && mv_nr > 0 {
            let value = -self.search(
                board,
                depth - Depth::new(1),
                -alpha - Value::new(1),
                -alpha,
                ply + Ply::new(1),
            );

            if value <= alpha {
                return value;
            }
        }

        -self.search(
            board,
            depth - Depth::new(1),
            -beta,
            -alpha,
            ply + Ply::new(1),
        )
    }

    fn search(
        &mut self,
        board: &Board,
        depth: Depth,
        mut alpha: Value,
        mut beta: Value,
        ply: Ply,
    ) -> Value {
        if ply == Ply::MAX || self.should_stop() {
            return board.evaluate();
        }

        if self.is_draw(board, ply) {
            return Value::DRAW;
        }

        // Check the transposition table for a stored value before we do anything else.
        let alpha_orig = alpha;
        let tt_move = {
            if let Some((mv, bound, value)) = self.transposition_table.read().unwrap().get(
                self.stack_state(ply).zobrist,
                depth,
                ply,
            ) {
                if bound & Bound::Lower && alpha < value {
                    alpha = value;
                }
                if bound & Bound::Upper && value < beta {
                    beta = value;
                }

                if alpha >= beta {
                    return value;
                }

                mv
            } else {
                None
            }
        };

        if depth == Depth::new(0) {
            return self.quiescence_search(board, alpha, beta, ply);
        }

        self.logger.update_search(ply);

        let moves = MoveVec::from(MoveGen::new_legal(board))
            .mvv_lva_rated(board)
            .sort_with_preference(
                [
                    tt_move,
                    self.stack_state(ply).killers[0],
                    self.stack_state(ply).killers[1],
                ]
                .iter()
                .filter_map(|x| *x),
            );
        let possible_move_count = moves.len();

        let mut best_value = -Value::INFINITE;
        let mut best_move = None;
        let mut new_board = *board;

        for (mv_nr, mv) in moves.iter().map(|entry| entry.mv).enumerate() {
            self.make_move(board, mv, &mut new_board, ply);

            let value = self.pv_search(&new_board, depth, alpha, beta, ply, mv_nr);

            if self.should_stop() {
                // If we're stopping, we don't trust the value, because it was likely cut off.
                // Rely on whatever we've found so far on only.
                return best_value;
            }

            if value > best_value {
                best_value = value;
                best_move = Some(mv);

                if value > alpha {
                    alpha = value;
                    if value >= beta {
                        break;
                    }
                }
            }
        }

        let bound = if possible_move_count == 0 {
            best_value = if board.in_check() {
                Value::mated_in(ply)
            } else {
                Value::DRAW
            };

            Bound::Exact
        } else if best_value <= alpha_orig {
            Bound::Upper
        } else if best_value >= beta {
            Bound::Lower
        } else {
            Bound::Exact
        };

        self.stack_state_mut(ply).update_killer(best_move);
        self.transposition_table.write().unwrap().store(
            self.stack_state(ply).zobrist,
            best_move,
            bound,
            best_value,
            depth,
            ply,
        );

        best_value
    }

    fn quiescence_search(
        &mut self,
        board: &Board,
        mut alpha: Value,
        beta: Value,
        ply: Ply,
    ) -> Value {
        self.logger.update_search(ply);

        if ply == Ply::MAX || self.should_stop() {
            return board.evaluate();
        }

        if self.is_draw(board, ply) {
            return Value::DRAW;
        }

        let mut best_value = -Value::INFINITE;

        // Stand pat:
        if !board.in_check() {
            best_value = board.evaluate();

            if best_value > alpha {
                alpha = best_value;

                if best_value >= beta {
                    return best_value;
                }
            }
        }

        let mut moves = MoveGen::new_legal(board);

        if moves.len() == 0 {
            if board.in_check() {
                return Value::mated_in(ply);
            }
            return Value::DRAW;
        }

        // If we're not in check, we only search captures.
        // In check we should also consider evasions.
        if !board.in_check() {
            let targets = board.color_combined(!board.side_to_move());
            moves.set_iterator_mask(*targets);
        }

        let moves = MoveVec::from(moves).mvv_lva_rated(board).sorted();

        let mut new_board = *board;

        for mv in moves.iter().map(|entry| entry.mv) {
            self.make_move(board, mv, &mut new_board, ply);

            let value = -self.quiescence_search(&new_board, -beta, -alpha, ply + Ply::new(1));

            if self.should_stop() {
                // If we're stopping, we don't trust the value, because it was likely cut off.
                // Rely on whatever we've found so far on only.
                return best_value;
            }

            best_value = best_value.max(value);

            if value > alpha {
                alpha = value;

                if value >= beta {
                    break;
                }
            }
        }

        best_value
    }
}
