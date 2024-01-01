#![allow(dead_code)]
use chess::{Board, ChessMove, Game, MoveGen};

use crate::evaluate::Evaluate;
use crate::limits::Limits;
use crate::movelist::{MoveEntry, MoveVec};
use crate::stop_signal::StopSignal;
use crate::value::{self, Depth, Value};
// use uci::{Info, InfoPart};

#[derive(Debug)]
pub struct Searcher {
    game: Game,
    limits: Limits,
    info_writer: uci::InfoWriter,
    stop_signal: StopSignal,
    root_moves: MoveVec,
}

impl Searcher {
    pub fn new(
        game: Game,
        options: Vec<uci::GoOption>,
        info_writer: uci::InfoWriter,
        stop_signal: StopSignal,
    ) -> Self {
        let root_moves = MoveVec::new_from_moves(MoveGen::new_legal(&game.current_position()));
        Self {
            game,
            limits: Limits::from(options.as_slice()),
            info_writer,
            stop_signal,
            root_moves,
        }
    }

    pub fn run(&mut self) -> ChessMove {
        assert!(
            !self.root_moves.is_empty(),
            "no legal moves in starting position, uci-crate promise violation"
        );

        for depth in 1..=self.limits.depth {
            if self.stop_signal.check() {
                break;
            }

            self.search_root(depth, -value::INFINITE, value::INFINITE);

            self.root_moves.sort_moves();
        }

        self.root_moves[0].mv
    }

    fn search_root(&mut self, depth: Depth, mut alpha: Value, beta: Value) {
        assert!(depth > 0, "depth must be greater than 0");

        // Reset all values, so the best move as determined at this new (larger) depth is pushed to the front
        self.root_moves.iter_mut().for_each(|m| {
            *m = MoveEntry::new(m.mv);
        });

        let root_board = self.game.current_position();
        let mut board = root_board;
        let mut best_value = -value::INFINITE;

        for (mv_nr, mv) in self.root_moves.clone().iter().enumerate() {
            root_board.make_move(mv.mv, &mut board);

            let value = self.pv_search(&board, depth, alpha, beta, 0, mv_nr);

            if self.stop_signal.check() {
                // If we're stopping, we don't trust the value, because it was likely cut off.
                // Rely on whatever we've found so far on only.
                return;
            }

            best_value = best_value.max(value);

            if value > alpha {
                alpha = value;

                self.root_moves[mv_nr].value = value;

                if value >= beta {
                    return;
                }
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
        ply: Depth,
        mv_nr: usize,
    ) -> Value {
        if depth > 1 && mv_nr > 0 {
            let value = -self.search(board, depth - 1, -alpha - 1, -alpha, ply + 1);

            if value <= alpha {
                return value;
            }
        }

        -self.search(board, depth - 1, -beta, -alpha, ply + 1)
    }

    fn search(
        &mut self,
        board: &Board,
        depth: Depth,
        mut alpha: Value,
        beta: Value,
        ply: Depth,
    ) -> Value {
        if self.stop_signal.check() || ply == value::MAX_PLY {
            return board.evaluate();
        }

        // TODO: Halfmove clock, repetition and insufficient material draw detection.

        if depth == 0 {
            return self.quiescence_search(board, alpha, beta, ply);
        }

        let moves = MoveGen::new_legal(board);
        if moves.len() == 0 {
            if *board.checkers() == chess::EMPTY {
                return value::DRAW;
            } else {
                return -value::CHECKMATE + Value::from(ply);
            }
        }

        let mut best_value = -value::INFINITE;
        let mut new_board = *board;

        for (mv_nr, mv) in moves.enumerate() {
            board.make_move(mv, &mut new_board);

            let value = self.pv_search(&new_board, depth, alpha, beta, ply, mv_nr);

            if self.stop_signal.check() {
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

    fn quiescence_search(
        &mut self,
        board: &Board,
        mut alpha: Value,
        beta: Value,
        ply: Depth,
    ) -> Value {
        if self.stop_signal.check() || ply == value::MAX_PLY {
            return board.evaluate();
        }

        // TODO: Halfmove clock, repetition and insufficient material draw detection.

        let mut best_value = -value::INFINITE;

        // Stand pat:
        if *board.checkers() == chess::EMPTY {
            best_value = board.evaluate();

            if best_value > alpha {
                alpha = best_value;

                if best_value >= beta {
                    return best_value;
                }
            }
        }

        let moves = MoveGen::new_legal(board);

        if moves.len() == 0 {
            if *board.checkers() == chess::EMPTY {
                return value::DRAW;
            } else {
                return -value::CHECKMATE + Value::from(ply);
            }
        }

        let targets = board.color_combined(!board.side_to_move());
        let mut captures = moves; // Rename to captures
        captures.set_iterator_mask(*targets);

        let mut new_board = *board;

        for mv in captures {
            board.make_move(mv, &mut new_board);

            let value = -self.quiescence_search(&new_board, -beta, -alpha, ply + 1);

            if self.stop_signal.check() {
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
