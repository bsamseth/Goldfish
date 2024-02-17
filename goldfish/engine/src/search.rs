use std::sync::{Arc, RwLock};

use chess::{Board, ChessMove, MoveGen};
use uci::UciPosition;

use crate::board::BoardExt;
use crate::evaluate::Evaluate;
use crate::limits::Limits;
use crate::logger::Logger;
use crate::movelist::{MoveEntry, MoveVec};
use crate::newtypes::{Depth, Ply, Value};
use crate::stop_signal::StopSignal;
use crate::tt::{Bound, TranspositionTable};

#[derive(Debug)]
pub struct Searcher {
    root_position: Board,
    ss: [StackState; Ply::MAX.as_usize() + 1],
    limits: Limits,
    logger: Logger,
    stop_signal: StopSignal,
    root_moves: MoveVec,
    transposition_table: Arc<RwLock<TranspositionTable>>,
}

#[derive(Debug, Clone, Copy, Default)]
pub struct StackState {
    pub halfmove_clock: usize,
    pub zobrist: u64,
}

impl Searcher {
    pub fn new(
        position: UciPosition,
        options: &[uci::GoOption],
        stop_signal: StopSignal,
        transposition_table: Arc<RwLock<TranspositionTable>>,
    ) -> Self {
        let mut board = position.start_pos;
        let mut root_position = board;
        let mut halfmove_clock = position.starting_halfmove_clock;
        for mv in position.moves {
            if board.halfmove_reset(mv) {
                halfmove_clock = 0;
            } else {
                halfmove_clock += 1;
            }
            board.make_move(mv, &mut root_position);
            board = root_position;
        }

        let mut stack_states = [StackState::default(); Ply::MAX.as_usize() + 1];
        stack_states[0] = StackState {
            halfmove_clock,
            zobrist: root_position.get_hash(),
        };

        let root_moves: MoveVec = MoveGen::new_legal(&root_position).into();

        Self {
            root_position,
            ss: stack_states,
            limits: Limits::from(options),
            logger: Logger::new(),
            stop_signal,
            root_moves: root_moves.mvv_lva_rated(&root_position).sorted(0),
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
    ///
    /// The PV will contain at most `depth` moves, as any more than this would likely be speculative.
    fn build_pv(&self, depth: Depth) -> Vec<ChessMove> {
        let mut pv = Vec::new();
        let mut board = self.root_position;

        // The root position is _not_ in the table, so we manually pick out the best root move.
        // The root move list is only sorted by the previous depth's values, so we must determine
        // it on demand.
        let first_move = self.root_moves.iter().max_by_key(|r| r.value).unwrap().mv;
        board = board.make_move_new(first_move);
        pv.push(first_move);

        while let Some((Some(mv), Bound::Exact, _)) = self.transposition_table.read().unwrap().get(
            board.get_hash(),
            Depth::new(0),
            Ply::new(0),
        ) {
            pv.push(mv);
            board = board.make_move_new(mv);

            if pv.len() >= depth.as_usize() {
                break;
            }
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
                self.ss[ply.as_usize()].zobrist,
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
            .with_move_at(0, tt_move)
            .sorted(tt_move.map_or(0, |_| 1));
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

        self.transposition_table.write().unwrap().store(
            self.ss[ply.as_usize()].zobrist,
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

        let moves = MoveVec::from(moves).mvv_lva_rated(board).sorted(0);

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

    /// Make a move on a pre-allocated board, and update the stack state.
    ///
    /// The move must be valid for [`board`], and [`result`] will be the result of
    /// applying [`mv`] to [`board`].
    ///
    /// [`ply`] should be the current ply count in the search, and _not_ the count after the move
    /// is made. I.e. this should be true: [`self.ss[ply].zobrist == board.get_hash()`].
    fn make_move(&mut self, board: &Board, mv: ChessMove, result: &mut Board, ply: Ply) {
        let current_halfmove = self.ss[ply.as_usize()].halfmove_clock;
        let new_ss = &mut self.ss[ply.as_usize() + 1];

        if board.halfmove_reset(mv) {
            new_ss.halfmove_clock = 0;
        } else {
            new_ss.halfmove_clock = current_halfmove + 1;
        }
        board.make_move(mv, result);
        new_ss.zobrist = result.get_hash();
    }

    fn is_draw(&self, board: &Board, ply: Ply) -> bool {
        let ply = ply.as_usize();
        if self.ss[ply].halfmove_clock >= 100 || board.has_insufficient_material() {
            return true;
        }

        // Check for repetition.
        // Check positions from the last halfmove clock reset, and return true if we've seen the
        // same position twice before. Positions are treated as equal by their Zobrist keys.
        self.ss[ply.saturating_sub(self.ss[ply].halfmove_clock)..ply]
            .iter()
            .filter({
                let latest_zobrist = self.ss[ply].zobrist;
                move |s| s.zobrist == latest_zobrist
            })
            .count()
            >= 2
    }
}
