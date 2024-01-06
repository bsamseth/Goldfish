use chess::{Board, ChessMove, Game, MoveGen};
use std::sync::{Arc, RwLock};

use crate::evaluate::Evaluate;
use crate::limits::Limits;
use crate::logger::Logger;
use crate::movelist::{MoveEntry, MoveVec};
use crate::stop_signal::StopSignal;
use crate::tt::{Bound, TranspositionTable};
use crate::value::{self, Depth, Value, MAX_PLY};

#[derive(Debug)]
pub struct Searcher {
    game: Game,
    limits: Limits,
    logger: Logger,
    stop_signal: StopSignal,
    root_moves: MoveVec,
    pv: Vec<Vec<ChessMove>>,
    transposition_table: Arc<RwLock<TranspositionTable>>,
}

impl Searcher {
    pub fn new(
        game: Game,
        options: Vec<uci::GoOption>,
        info_writer: uci::InfoWriter,
        stop_signal: StopSignal,
        transposition_table: Arc<RwLock<TranspositionTable>>,
    ) -> Self {
        let root_moves = MoveVec::new_from_moves(MoveGen::new_legal(&game.current_position()));
        let pv = (0..MAX_PLY)
            .map(|_| Vec::with_capacity(MAX_PLY as usize))
            .collect();
        Self {
            game,
            limits: Limits::from(options.as_slice()),
            logger: Logger::new(info_writer),
            stop_signal,
            root_moves,
            pv,
            transposition_table,
        }
    }

    /// Update the principal variation.
    ///
    /// Set the principal variation move at `ply`  to `[mv, ..pv[ply+1]]`.
    /// That is, the pv at `ply` starts with `mv`, and is followed by the pv at `ply+1`.
    ///
    /// See: https://www.chessprogramming.org/Principal_Variation#PV-List_on_the_Stack
    /// This is essentially the same, but the pvs are preallocated.
    fn update_pv(&mut self, mv: ChessMove, ply: Depth) {
        self.pv[ply as usize].clear();
        self.pv[ply as usize].push(mv);

        // Use split_at_mut to allow having a mutable reference to the pv[ply] while
        // having a shared reference to pv[ply+1].
        let (dst, src) = self.pv.split_at_mut(ply as usize + 1);
        let (dst, src) = (&mut dst[ply as usize], &src[0]);
        dst.extend_from_slice(src);
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

            self.logger.set_current_depth(depth);
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
            self.logger.set_current_move(mv.mv, mv_nr + 1);
            self.logger.send_status();

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

                self.update_pv(mv.mv, 0);
                self.logger.send_move(&self.root_moves[mv_nr], &self.pv[0]);
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
        mut beta: Value,
        ply: Depth,
    ) -> Value {
        if self.stop_signal.check() || ply == value::MAX_PLY {
            return board.evaluate();
        }

        // TODO: Halfmove clock, repetition and insufficient material draw detection.

        let alpha_orig = alpha;
        let tt_entry = self
            .transposition_table
            .read()
            .unwrap()
            .get(board.get_hash(), depth);
        if let Some((mv, bound, value)) = tt_entry {
            if bound & Bound::Lower && alpha < value {
                alpha = value;
                if let Some(mv) = mv {
                    self.update_pv(mv, ply);
                }
            }
            if bound & Bound::Upper && value < beta {
                beta = value;
            }

            if alpha >= beta {
                return value;
            }
        }

        if depth == 0 {
            return self.quiescence_search(board, alpha, beta, ply);
        }

        self.logger.update_search(ply);

        let moves = MoveGen::new_legal(board);
        let possible_move_count = moves.len();

        let mut best_value = -value::INFINITE;
        let mut best_move = None;
        let mut new_board = *board;

        for (mv_nr, mv) in moves.enumerate() {
            board.make_move(mv, &mut new_board);

            let value = self.pv_search(&new_board, depth, alpha, beta, ply, mv_nr);

            if self.stop_signal.check() {
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
                    self.update_pv(mv, ply);
                }
            }
        }

        let bound = if possible_move_count == 0 {
            best_value = if *board.checkers() == chess::EMPTY {
                value::DRAW
            } else {
                -value::CHECKMATE + Value::from(ply)
            };

            Bound::Exact
        } else if best_value <= alpha_orig {
            Bound::Upper
        } else if best_value >= beta {
            Bound::Lower
        } else {
            Bound::Exact
        };

        self.transposition_table.write().unwrap().store(
            board.get_hash(),
            best_move,
            bound,
            best_value,
            depth,
        );

        best_value
    }

    fn quiescence_search(
        &mut self,
        board: &Board,
        mut alpha: Value,
        beta: Value,
        ply: Depth,
    ) -> Value {
        self.logger.update_search(ply);

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
