use chess::{Board, ChessMove, Game, MoveGen};
use std::sync::{Arc, RwLock};

use crate::evaluate::Evaluate;
use crate::limits::Limits;
use crate::logger::Logger;
use crate::movelist::{MoveEntry, MoveVec};
use crate::stop_signal::StopSignal;
use crate::tt::{Bound, TranspositionTable};
use crate::value::{self, Depth, Value};

#[derive(Debug)]
pub struct Searcher {
    game: Game,
    root_position: Board,
    limits: Limits,
    logger: Logger,
    stop_signal: StopSignal,
    root_moves: MoveVec,
    transposition_table: Arc<RwLock<TranspositionTable>>,
}

impl Searcher {
    pub fn new(
        game: Game,
        options: &[uci::GoOption],
        stop_signal: StopSignal,
        transposition_table: Arc<RwLock<TranspositionTable>>,
    ) -> Self {
        let root_moves = MoveVec::new_from_moves(MoveGen::new_legal(&game.current_position()));
        let root_position = game.current_position();
        Self {
            game,
            root_position,
            limits: Limits::from(options),
            logger: Logger::new(),
            stop_signal,
            root_moves,
            transposition_table,
        }
    }

    fn should_stop(&self) -> bool {
        self.stop_signal.check()
            || self.logger.search_start_time.elapsed().as_millis() as usize >= self.limits.movetime
            || self.logger.total_nodes >= self.limits.nodes
    }

    /// Returns the principal variation for the current position.
    ///
    /// The principal variation is the sequence of moves that the engine thinks is the best.
    /// It is used to display the engine's thinking process.
    ///
    /// We use the transposition table to build the PV on demand. This is not the most efficient
    /// way to build the PV itself, but it removes complexity (and cycles) from the main search
    /// code path. Only when a new best move is found at the root (for a given depth) do we need
    /// to build the PV, which means this doesn't impact the search speed.
    ///
    /// Another potential downside is that the PV might be cut short if a replacement occurs in the
    /// table. For a reasonable table size this should be rare, so this approach is the less
    /// intrusive one.
    fn build_pv(&self) -> Vec<ChessMove> {
        let mut pv = Vec::new();
        let mut board = self.root_position;

        // The root position is _not_ in the table, so we manually pick out the best root move.
        // The root move list is only sorted by the previous depth's values, so we must determine
        // it on demand.
        let first_move = self.root_moves.iter().max_by_key(|r| r.value).unwrap().mv;
        board = board.make_move_new(first_move);
        pv.push(first_move);

        while let Some((Some(mv), _, _)) = self
            .transposition_table
            .read()
            .unwrap()
            .get(board.get_hash(), 0)
        {
            pv.push(mv);
            board = board.make_move_new(mv);
        }

        pv
    }

    /// Run a search and return the best move.
    ///
    /// The search may stop for a variety of reasons, depending on the options set by the user,
    /// or if the user sends a stop signal. Still, it should always eventually return.
    pub fn run(&mut self) -> ChessMove {
        assert!(
            !self.root_moves.is_empty(),
            "no legal moves in starting position, uci-crate promise violation"
        );

        for depth in 1..=self.limits.depth {
            if self.should_stop() {
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

            if self.should_stop() {
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

                self.logger
                    .send_move(&self.root_moves[mv_nr], &self.build_pv());
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
        if ply == value::MAX_PLY || self.should_stop() {
            return board.evaluate();
        }

        // TODO: Halfmove clock, repetition and insufficient material draw detection.

        let alpha_orig = alpha;
        let tt_entry = self
            .transposition_table
            .read()
            .unwrap()
            .get(board.get_hash(), depth);
        if let Some((_, bound, value)) = tt_entry {
            if bound & Bound::Lower && alpha < value {
                alpha = value;
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

        if ply == value::MAX_PLY || self.should_stop() {
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
            }
            return -value::CHECKMATE + Value::from(ply);
        }

        let targets = board.color_combined(!board.side_to_move());
        let mut captures = moves; // Rename to captures
        captures.set_iterator_mask(*targets);

        let mut new_board = *board;

        for mv in captures {
            board.make_move(mv, &mut new_board);

            let value = -self.quiescence_search(&new_board, -beta, -alpha, ply + 1);

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
